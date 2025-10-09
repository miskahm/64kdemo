@echo off
echo Cleaning 64K Demo project...

REM Remove build directory
if exist vulkan-demo\build (
    echo Removing vulkan-demo\build...
    rmdir /s /q vulkan-demo\build
)

REM Remove test files
if exist test.c del /q test.c
if exist vulkan-demo\test.c del /q vulkan-demo\test.c
if exist vulkan-demo\manual_build\test.c del /q vulkan-demo\manual_build\test.c

REM Remove temporary output files
if exist vulkan-demo\temp_output.txt del /q vulkan-demo\temp_output.txt
if exist temp_output.txt del /q temp_output.txt

REM Remove test batch files
if exist vulkan-demo\test_run.bat del /q vulkan-demo\test_run.bat
if exist vulkan-demo\run_test.bat del /q vulkan-demo\run_test.bat

REM Remove debug/fix documentation
if exist vulkan-demo\AUDIO_REACTIVITY_FIX.md del /q vulkan-demo\AUDIO_REACTIVITY_FIX.md
if exist vulkan-demo\CRASH_DEBUG.md del /q vulkan-demo\CRASH_DEBUG.md
if exist vulkan-demo\CRASH_FIX_NOTES.md del /q vulkan-demo\CRASH_FIX_NOTES.md
if exist vulkan-demo\FINAL_FIX.md del /q vulkan-demo\FINAL_FIX.md
if exist vulkan-demo\FINAL_STATUS.md del /q vulkan-demo\FINAL_STATUS.md
if exist vulkan-demo\FIXED.md del /q vulkan-demo\FIXED.md
if exist vulkan-demo\INVESTIGATION_RESULTS.md del /q vulkan-demo\INVESTIGATION_RESULTS.md

REM Remove agent documentation
if exist AGENTS.md del /q AGENTS.md
if exist subagent_guide.md del /q subagent_guide.md

REM Remove .opencode directory
if exist .opencode (
    echo Removing .opencode...
    rmdir /s /q .opencode
)

REM Remove package-lock.json if exists
if exist package-lock.json del /q package-lock.json

echo.
echo Cleanup complete!
echo.
echo Ready for git commit. Run:
echo   git add .
echo   git status
echo   git commit -m "Initial commit: 64K Vulkan demoscene intro"
