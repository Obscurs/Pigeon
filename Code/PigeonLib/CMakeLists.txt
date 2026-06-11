file(GLOB ROOT
    "Pigeon.h"
)

file(GLOB PGAUDIO
    "Pigeon/Audio/AudioDevice.cpp"
    "Pigeon/Audio/AudioDevice.h"
    "Pigeon/Audio/AudioDeviceSingletonComponent.h"
    "Pigeon/Audio/AudioPlaybackSystem.cpp"
    "Pigeon/Audio/AudioPlaybackSystem.h"
    "Pigeon/Audio/AudioVolume.h"
    "Pigeon/Audio/AudioVolumeSingletonComponent.h"
    "Pigeon/Audio/AudioVolumeSystem.cpp"
    "Pigeon/Audio/AudioVolumeSystem.h"
    "Pigeon/Audio/EAudioCategory.h"
    "Pigeon/Audio/PauseAudioRequestOneFrameComponent.h"
    "Pigeon/Audio/PlayAudioRequestOneFrameComponent.h"
    "Pigeon/Audio/ResumeAudioRequestOneFrameComponent.h"
    "Pigeon/Audio/SetAudioVolumeRequestOneFrameComponent.h"
    "Pigeon/Audio/SoundClip.cpp"
    "Pigeon/Audio/SoundClip.h"
    "Pigeon/Audio/StopAudioRequestOneFrameComponent.h"
)

file(GLOB PGCORE
    "Pigeon/Core/Application.cpp"
    "Pigeon/Core/Clock.h"
    "Pigeon/Core/ConfigLoaderSystem.cpp"
    "Pigeon/Core/Core.h"
    "Pigeon/Core/EWindowMode.h"
    "Pigeon/Core/EngineConfigSingletonComponent.h"
    "Pigeon/Core/EntryPoint.h"
    "Pigeon/Core/FileUtils.cpp"
    "Pigeon/Core/FileUtils.h"
    "Pigeon/Core/EventComponent.h"
    "Pigeon/Core/Input.h"
    "Pigeon/Core/InputStateSingletonComponent.h"
    "Pigeon/Core/InputSystem.cpp"
    "Pigeon/Core/KeyCodes.h"
    "Pigeon/Core/KeyPressedEventComponent.h"
    "Pigeon/Core/KeyReleasedEventComponent.h"
    "Pigeon/Core/KeyTypedEventComponent.h"
    "Pigeon/Core/Layer.cpp"
    "Pigeon/Core/LayerStack.cpp"
    "Pigeon/Core/Log.cpp"
    "Pigeon/Core/MouseButtonPressedEventComponent.h"
    "Pigeon/Core/MouseButtonReleasedEventComponent.h"
    "Pigeon/Core/MouseMovedEventComponent.h"
    "Pigeon/Core/MouseScrolledEventComponent.h"
    "Pigeon/Core/ResourceManagerSystem.cpp"
    "Pigeon/Core/ResourceMapSingletonComponent.h"
    "Pigeon/Core/SaveDataSingletonComponent.h"
    "Pigeon/Core/SaveDataSystem.cpp"
    "Pigeon/Core/SaveDataSystem.h"
    "Pigeon/Core/SetSaveDataRequestOneFrameComponent.h"
    "Pigeon/Core/SetWindowResolutionRequestOneFrameComponent.h"
    "Pigeon/Core/Timestep.h"
    "Pigeon/Core/UUID.cpp"
    "Pigeon/Core/Window.h"
    "Pigeon/Core/WindowCloseEventComponent.h"
    "Pigeon/Core/WindowConfigSingletonComponent.h"
    "Pigeon/Core/WindowConfigSystem.cpp"
    "Pigeon/Core/WindowConfigSystem.h"
    "Pigeon/Core/WindowResizeEventComponent.h"
)

file(GLOB PGIMGUI
    "Pigeon/ImGui/ImGuiLayer.cpp"
)

file(GLOB RENDERER
    "Pigeon/Renderer/Buffer.cpp"
    "Pigeon/Renderer/CameraSystem.cpp"
    "Pigeon/Renderer/DrawQuadInFrameEvent.h"
    "Pigeon/Renderer/DrawSpriteInFrameEvent.h"
    "Pigeon/Renderer/DrawStringInFrameEvent.h"
    "Pigeon/Renderer/DrawUIQuadInFrameEvent.h"
    "Pigeon/Renderer/DrawUIStringInFrameEvent.h"
    "Pigeon/Renderer/Font.cpp"
    "Pigeon/Renderer/GraphicsContext.cpp"
    "Pigeon/Renderer/MSDFData.h"
    "Pigeon/Renderer/OrthographicCamera.cpp"
    "Pigeon/Renderer/OrthographicCameraComponent.h"
    "Pigeon/Renderer/RenderCommand.cpp"
    "Pigeon/Renderer/Renderer.cpp"
    "Pigeon/Renderer/Renderer2DSystem.cpp"
    "Pigeon/Renderer/RendererAPI.cpp"
    "Pigeon/Renderer/RendererDataSingletonComponent.h"
    "Pigeon/Renderer/SetCameraRequestOneFrameComponent.h"
    "Pigeon/Renderer/SetSpriteAnimationRequestOneFrameComponent.h"
    "Pigeon/Renderer/Shader.cpp"
    "Pigeon/Renderer/Sprite.cpp"
    "Pigeon/Renderer/SpriteAnimationComponent.h"
    "Pigeon/Renderer/SpriteAnimationSystem.cpp"
    "Pigeon/Renderer/SpriteComponent.h"
    "Pigeon/Renderer/SpriteRenderSystem.cpp"
    "Pigeon/Renderer/SpriteSheet.cpp"
    "Pigeon/Renderer/SpriteSheet.h"
    "Pigeon/Renderer/Texture.cpp"
)

file(GLOB UI
    "Pigeon/UI/UIComponents.h"
    "Pigeon/UI/UIControlSystem.cpp"
    "Pigeon/UI/UIEventSystem.cpp"
    "Pigeon/UI/UIHelpers.cpp"
    "Pigeon/UI/UIRenderSystem.cpp"
)

file(GLOB TRANSFORM
    "Pigeon/Transform/CameraTransformRequestOneFrameComponent.h"
    "Pigeon/Transform/LocalBoundsComponent.h"
    "Pigeon/Transform/PositionComponent.h"
    "Pigeon/Transform/ResolvedTransformRequestOneFrameComponent.h"
    "Pigeon/Transform/RotationComponent.h"
    "Pigeon/Transform/ScaleComponent.h"
    "Pigeon/Transform/TransformComposeSystem.cpp"
    "Pigeon/Transform/TransformRequestData.h"
    "Pigeon/Transform/TransformResolveSystem.cpp"
    "Pigeon/Transform/WorldTransformComponent.h"
)

file(GLOB ECS_WORLD
    "Pigeon/ECS/CheckedRegistryAccessor.h"
    "Pigeon/ECS/Entity.h"
    "Pigeon/ECS/System.h"
    "Pigeon/ECS/World.cpp"
    "Pigeon/ECS/World.h"
)

file(GLOB ECS_SYSTEMS)

file(GLOB ECS_COMPONENTS)

file(GLOB PLATFORM_WIN
    "Platform/Windows/WindowsInput.cpp"
    "Platform/Windows/WindowsInputKeyCodeMapping.h"
    "Platform/Windows/WindowsWindow.cpp"
)

file(GLOB PLATFORM_DX
    "Platform/DirectX11/Dx11Buffer.cpp"
    "Platform/DirectX11/Dx11Context.cpp"
    "Platform/DirectX11/Dx11RendererAPI.cpp"
    "Platform/DirectX11/Dx11Shader.cpp"
    "Platform/DirectX11/Dx11Texture.cpp"
)

file(GLOB PLATFORM_MINIAUDIO
    "Platform/Miniaudio/MiniaudioAudioDevice.cpp"
    "Platform/Miniaudio/MiniaudioAudioDevice.h"
)

file(GLOB PLATFORM_TESTING
    "Platform/Testing/TestingBuffer.cpp"
    "Platform/Testing/TestingContext.cpp"
    "Platform/Testing/TestingHelper.cpp"
    "Platform/Testing/TestingInput.cpp"
    "Platform/Testing/TestingRendererAPI.cpp"
    "Platform/Testing/TestingShader.cpp"
    "Platform/Testing/TestingTexture.cpp"
    "Platform/Testing/TestingWindow.cpp"
)

file(GLOB VENDOR
    "vendor/miniaudio/miniaudio.cpp"
    "vendor/stb_image/stb_image.cpp"
)

file(GLOB_RECURSE SRC
    "Pigeon/*.cpp"
    "Pigeon/*.h"
    "Platform/*.cpp"
    "Platform/*.h"
    "ECS/*.cpp"
    "ECS/*.h"
    "ECS/Systems/*.cpp"
    "ECS/Components/*.h"
    "vendor/miniaudio/*.cpp"
    "vendor/miniaudio/*.h"
    "vendor/stb_image/*.cpp"
    "vendor/stb_image/*.h"
)

source_group("" FILES ${ROOT})
source_group("Pigeon\\Audio" FILES ${PGAUDIO})
source_group("Pigeon\\Core" FILES ${PGCORE})
source_group("Pigeon\\ImGui" FILES ${PGIMGUI})
source_group("Pigeon\\Renderer" FILES ${RENDERER})
source_group("Pigeon\\UI" FILES ${UI})
source_group("Pigeon\\Transform" FILES ${TRANSFORM})
source_group("Platform\\Testing" FILES ${PLATFORM_TESTING})
source_group("Platform\\DirectX11" FILES ${PLATFORM_DX})
source_group("Platform\\Miniaudio" FILES ${PLATFORM_MINIAUDIO})
source_group("Pigeon\\ECS" FILES ${ECS_WORLD})
source_group("Pigeon\\ECS\\Systems" FILES ${ECS_SYSTEMS})
source_group("Pigeon\\ECS\\Components" FILES ${ECS_COMPONENTS})
source_group("vendor" FILES ${VENDOR})
# Properties->C/C++->General->Additional Include Directories
include_directories (
    "${PROJECT_SOURCE_DIR}/Code/PigeonLib"
)

# Set Properties->General->Configuration Type to Application(.exe)
# Creates app.exe with the listed sources (main.cxx)
# Adds sources to the Solution Explorer
add_library(PigeonLib STATIC)

# Add common sources to target
target_sources(PigeonLib PRIVATE ${SRC})

# Platform/Windows files are included for configurations other than "Test"
# This assumes WIN32 is true; adjust as necessary for other platforms
if(WIN32)
    target_sources(PigeonLib PRIVATE ${PLATFORM_WIN})
    source_group("Platform\\Windows" FILES ${PLATFORM_WIN})
endif()

set_property(TARGET PigeonLib PROPERTY ENABLE_EXPORTS 1)
set_property(TARGET PigeonLib PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/Data)

target_precompile_headers(PigeonLib PRIVATE pch.h)

# Properties->Linker->Input->Additional Dependencies

if(WIN32)
    target_compile_definitions(PigeonLib PRIVATE PG_PLATFORM_WINDOWS)
    target_compile_definitions(PigeonLib PRIVATE PG_BUILD_DLL)
elseif(UNIX)
    target_compile_definitions(PigeonLib PRIVATE PG_PLATFORM_UNIX)
endif()

target_link_libraries (
    PigeonLib
    imgui
    spdlog
    glm
    msdfgen
    msdf-atlas-gen
    nlohmann_json
)

target_include_directories(PigeonLib PUBLIC 
    ${ENTT_INCLUDE_DIRS}
    ${MSDFGEN_INCLUDE_DIRS}
)

# Properties->General->Output Directory
set_target_properties(PigeonLib PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

file(GLOB DLLS_TO_COPY
    ${PROJECT_SOURCE_DIR}/External
)
file(GLOB DATA_TO_COPY
    ${PROJECT_SOURCE_DIR}/Data
)

add_custom_command(TARGET PigeonLib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DLLS_TO_COPY}/$(Configuration)
    ${PROJECT_BINARY_DIR}/bin/$(Configuration)
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${DATA_TO_COPY}
    ${PROJECT_BINARY_DIR}/bin/$(Configuration)
    # SaveData sits beside the running exe (../SaveData from bin/$(Configuration)),
    # mirroring its source layout as a sibling of Data.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/SaveData
    ${PROJECT_BINARY_DIR}/bin/SaveData
)
