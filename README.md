# STM32F407ZET6 C++20 Coroutine Command Framework

This project is a bare-metal STM32F407ZE VS Code/GCC firmware skeleton.

中文编译调试说明见：[docs/vscode_build_debug_zh.md](docs/vscode_build_debug_zh.md)

工程简要说明见：[docs/project_brief_zh.md](docs/project_brief_zh.md)

代码风格说明见：[docs/coding_style_zh.md](docs/coding_style_zh.md)

项目对话记录与最终计划见：[docs/conversation_and_final_plan_zh.md](docs/conversation_and_final_plan_zh.md)

## Build

In VS Code:

- Press `Ctrl+Shift+B`, or
- Press `F1`, run `Tasks: Run Task`, then select `Build Firmware (GCC)`.

From a terminal:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1
```

If the GNU Arm toolchain is installed in another location, either edit
`.vscode/settings.json` or run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build.ps1 -Toolchain C:\SysGCC\arm-eabi\bin
```

Outputs are generated in `build/`:

- `firmware.elf`
- `firmware.hex`
- `firmware.bin`

## Debug

Use the VS Code configuration `J-Link Debug STM32F407ZE (CDT GDB)`.

The launch configuration runs the task `Debug: Build + Start J-Link GDB Server`.
That task builds the firmware, starts J-Link GDB Server on `localhost:2331`, waits
until the server log says it is ready for GDB, then the debugger connects to it.

This project uses the Eclipse CDT GDB debug adapter (`type: gdbtarget`) instead of
Microsoft `cppdbg`, because `cppdbg` can fail on this setup with an ARM
architecture parsing error.

If a stale server is left running, use `F1` -> `Tasks: Run Task` -> `Stop J-Link GDB Server`.

For another computer, configure only these paths in `.vscode/settings.json`:

- `stm32.gccPath`
- `stm32.jlinkGdbServer`

Useful watch expressions:

- `app::g_diagnostics`
- `app::g_diagnostics.m_loopCount`
- `app::g_diagnostics.m_commandStarted`
- `app::g_diagnostics.m_motorStarted`
- `app::g_diagnostics.m_motorCompleted`
- `app::g_diagnostics.m_ackOk`

The current demo injects one in-memory `MoveStep` frame at boot, simulates motor completion, then writes an ACK.
