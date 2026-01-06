# Manual Review Required

This repository contains AI-generated test code. **Human review is mandatory before any build/test execution.**

## Generated test files
- tests/test_Interlocking.cpp

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

Required approval files:
- tests/review/APPROVED.test_Interlocking.cpp.flag

Each approval file contents must be exactly:

approved = true
reviewed_by = <human_name>
date = <ISO date>
