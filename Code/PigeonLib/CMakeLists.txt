file(GLOB ROOT
    "Pigeon.h"
)

file(GLOB PGCORE
    "Pigeon/Core/Application.cpp"
    "Pigeon/Core/Core.h"
    "Pigeon/Core/Clock.h"
    "Pigeon/Core/EntryPoint.h"
    "Pigeon/Core/Input.h"
    "Pigeon/Core/KeyCodes.h"
    "Pigeon/Core/Layer.cpp"
    "Pigeon/Core/LayerStack.cpp"
    "Pigeon/Core/Log.cpp"
    "Pigeon/Core/MouseButtonCodes.h"
    "Pigeon/Core/OrthographicCameraController.cpp"
    "Pigeon/Core/Timestep.h"
    "Pigeon/Core/Window.h"
)

file(GLOB PGIMGUI
    "Pigeon/ImGui/ImGuiLayer.cpp"
)

file(GLOB RENDERER
    "Pigeon/Renderer/Buffer.cpp"
    "Pigeon/Renderer/GraphicsContext.cpp"
    "Pigeon/Renderer/OrthographicCamera.cpp"
    "Pigeon/Renderer/RenderCommand.cpp"
    "Pigeon/Renderer/Renderer.cpp"
    "Pigeon/Renderer/Renderer2D.cpp"
    "Pigeon/Renderer/RendererAPI.cpp"
    "Pigeon/Renderer/Shader.cpp"
    "Pigeon/Renderer/Sprite.cpp"
    "Pigeon/Renderer/Texture.cpp"
)

file(GLOB ECS_WORLD
    "Pigeon/ECS/System.h"
    "Pigeon/ECS/World.cpp"
)

file(GLOB ECS_SYSTEMS
    "Pigeon/ECS/Systems/TestSystem.cpp"
)

file(GLOB ECS_COMPONENTS
    "Pigeon/ECS/Components/TestComponent.h"
)

file(GLOB PLATFORM_WIN
    "Platform/Windows/WindowsWindow.cpp"
    "Platform/Windows/WindowsInput.cpp"
)

file(GLOB PLATFORM_DX
    "Platform/DirectX11/Dx11Buffer.cpp"
    "Platform/DirectX11/Dx11Context.cpp"
    "Platform/DirectX11/Dx11RendererAPI.cpp"
    "Platform/DirectX11/Dx11Shader.cpp"
    "Platform/DirectX11/Dx11Texture.cpp"
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
    "vendor/stb_image/*.cpp"
    "vendor/stb_image/*.h"
)

source_group("" FILES ${ROOT})
source_group("Pigeon\\Core" FILES ${PGCORE})
source_group("Pigeon\\ImGui" FILES ${PGIMGUI})
source_group("Pigeon\\Renderer" FILES ${RENDERER})
source_group("Platform\\Testing" FILES ${PLATFORM_TESTING})
source_group("Platform\\DirectX11" FILES ${PLATFORM_DX})
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
)

target_include_directories(PigeonLib PUBLIC ${ENTT_INCLUDE_DIRS})

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
)
