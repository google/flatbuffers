## Releasing

1. clean the git repo and update with the master branch.
2. run `./scripts/release.sh` to update the release versions.
3. rebuild everything `make clean && make -j`
4. make sure tests passes: `./flattests`
5. generate any code `./scripts/generate_code.py`
6. update the `CHANGELOG.md` file with the new release and any bullet points of things to highlight (this might require input from the admins).
7. `git add -u` make sure we are only checking in tracked files.
8. `git commit -m "FlatBuffers Version YY.MM.DD` and file a PR to approve.
9. `git tag -a -m "FlatBuffers Version YY.MM.DD" vYY.MM.DD `
10. `git push origin vYY.MM.DD`