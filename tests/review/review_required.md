# Manual Review Required

This repository contains AI-generated test code. **Human review is mandatory before any build/test execution.**

## Generated test files
- tests/src/logic/test_ControllerHelpers.cpp
- tests/src/logic/test_ControllerLogic.cpp
- tests/src/logic/test_Interlocking.cpp

## Skipped functions (with reasons)
- (none)

## Hardware dependencies detected
- (none)

## Known limitations / assumptions
- Generated tests are AI-produced and may contain incorrect assumptions; review is required.
- No compilation/build/test execution is performed until approval is recorded.
- Hardware-dependent behavior is not simulated; hardware-touching functions may be skipped or require stubs/mocks.

## Approval gate
Create an approval file for EACH generated test file before building or running tests:

Required approval files (preferred; mirrors project structure under tests/):
- tests/review/src/logic/test_ControllerHelpers.cpp.flag
- tests/review/src/logic/test_ControllerLogic.cpp.flag
- tests/review/src/logic/test_Interlocking.cpp.flag

Also accepted (back-compat):
- tests/review/tests/src/logic/test_ControllerHelpers.cpp.flag
- tests/review/tests/src/logic/test_ControllerLogic.cpp.flag
- tests/review/tests/src/logic/test_Interlocking.cpp.flag
- tests/review/APPROVED.test_ControllerHelpers.cpp.flag
- tests/review/APPROVED.test_ControllerLogic.cpp.flag
- tests/review/APPROVED.test_Interlocking.cpp.flag

Each approval file contents must be exactly:

approved = true
reviewed_by = <human_name>
date = <ISO date>
