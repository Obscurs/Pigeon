#!/usr/bin/env python3
"""
Convert a *standard single-type* diffusers SDXL ControlNet (.safetensors, tensors named
down_blocks.* / controlnet_cond_embedding.* / ...) into the original CompVis/lllyasviel layout
(input_blocks.* / zero_convs.* / input_hint_block.* / middle_block_out.*) that stable-diffusion.cpp
loads. Pure name remap + raw data copy; optionally casts F32 -> F16 (needs numpy) to halve VRAM.

Usage: python convert_controlnet_diffusers_to_original.py <in.safetensors> <out.safetensors> [--fp16]
"""
import json
import struct
import sys

try:
    import numpy as np
    HAVE_NUMPY = True
except Exception:
    HAVE_NUMPY = False

# (sd, hf): replace the hf substring with the sd substring.
TOPLEVEL = [
    ("time_embed.0", "time_embedding.linear_1"),
    ("time_embed.2", "time_embedding.linear_2"),
    ("label_emb.0.0", "add_embedding.linear_1"),
    ("label_emb.0.2", "add_embedding.linear_2"),
    ("input_blocks.0.0", "conv_in"),
]
RESNET = [
    ("in_layers.0", "norm1"),
    ("in_layers.2", "conv1"),
    ("out_layers.0", "norm2"),
    ("out_layers.3", "conv2"),
    ("emb_layers.1", "time_emb_proj"),
    ("skip_connection", "conv_shortcut"),
]
# SDXL UNet encoder: 3 levels, 2 layers each, attentions on levels 1&2, downsamplers on levels 0&1.
LAYER = []
for i in range(3):
    for j in range(2):
        LAYER.append((f"input_blocks.{3 * i + j + 1}.0.", f"down_blocks.{i}.resnets.{j}."))
        if i > 0:
            LAYER.append((f"input_blocks.{3 * i + j + 1}.1.", f"down_blocks.{i}.attentions.{j}."))
    if i < 2:
        LAYER.append((f"input_blocks.{3 * (i + 1)}.0.op.", f"down_blocks.{i}.downsamplers.0.conv."))
LAYER.append(("middle_block.1.", "mid_block.attentions.0."))
LAYER.append(("middle_block.0.", "mid_block.resnets.0."))
LAYER.append(("middle_block.2.", "mid_block.resnets.1."))


def convert_name(hf):
    if hf.startswith("controlnet_cond_embedding."):
        rest = hf[len("controlnet_cond_embedding."):]
        if rest.startswith("conv_in."):
            return "input_hint_block.0." + rest[len("conv_in."):]
        if rest.startswith("conv_out."):
            return "input_hint_block.14." + rest[len("conv_out."):]
        if rest.startswith("blocks."):
            p = rest.split(".")
            return f"input_hint_block.{2 + 2 * int(p[1])}." + ".".join(p[2:])
        return None
    if hf.startswith("controlnet_down_blocks."):
        p = hf[len("controlnet_down_blocks."):].split(".")
        return f"zero_convs.{int(p[0])}.0." + ".".join(p[1:])
    if hf.startswith("controlnet_mid_block."):
        return "middle_block_out.0." + hf[len("controlnet_mid_block."):]

    sd = hf
    if "resnets" in sd:
        for a, b in RESNET:
            sd = sd.replace(b, a)
    for a, b in LAYER:
        sd = sd.replace(b, a)
    for a, b in TOPLEVEL:
        sd = sd.replace(b, a)
    return sd


def main():
    src, dst = sys.argv[1], sys.argv[2]
    fp16 = "--fp16" in sys.argv[3:] and HAVE_NUMPY
    if "--fp16" in sys.argv[3:] and not HAVE_NUMPY:
        print("numpy not available; keeping original dtypes")

    with open(src, "rb") as f:
        header_len = struct.unpack("<Q", f.read(8))[0]
        header = json.loads(f.read(header_len))
    data_start = 8 + header_len

    tensors = [(k, v) for k, v in header.items() if k != "__metadata__"]
    # Read in source order so the copy is a sequential scan.
    tensors.sort(key=lambda kv: kv[1]["data_offsets"][0])

    new_header = {}
    plan = []  # (new_name, dtype, shape, src_begin, src_end, cast_to_f16)
    cursor = 0
    for name, info in tensors:
        new_name = convert_name(name)
        if new_name is None:
            raise SystemExit(f"unmapped tensor: {name}")
        begin, end = info["data_offsets"]
        dtype = info["dtype"]
        cast = fp16 and dtype == "F32"
        out_dtype = "F16" if cast else dtype
        out_len = (end - begin) // 2 if cast else (end - begin)
        new_header[new_name] = {"dtype": out_dtype, "shape": info["shape"], "data_offsets": [cursor, cursor + out_len]}
        plan.append((new_name, dtype, src_start := data_start + begin, data_start + end, cast))
        cursor += out_len

    header_bytes = json.dumps(new_header, separators=(",", ":")).encode("utf-8")
    pad = (-len(header_bytes)) % 8  # data must start on an 8-byte boundary
    header_bytes += b" " * pad

    with open(src, "rb") as fin, open(dst, "wb") as fout:
        fout.write(struct.pack("<Q", len(header_bytes)))
        fout.write(header_bytes)
        for new_name, dtype, begin, end, cast in plan:
            fin.seek(begin)
            raw = fin.read(end - begin)
            if cast:
                raw = np.frombuffer(raw, dtype=np.float32).astype(np.float16).tobytes()
            fout.write(raw)

    print(f"wrote {dst}: {len(new_header)} tensors, fp16={fp16}")


if __name__ == "__main__":
    main()
