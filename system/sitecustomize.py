import platform

# override platform.machine to return a fake value
def _fake_machine() -> str:
    return "x86_64"

platform.machine = _fake_machine
