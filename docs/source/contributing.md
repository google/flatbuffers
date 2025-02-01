# Contributing

We encourage community contributions to FlatBuffers through pull requests at the
main
[http://github.com/google/flatbuffers](http://github.com/google/flatbuffers)
repository.

!!! note

    The FlatBuffers project is not staffed by any full time Google employee, and
    is managed by a small team of 20%ers. So response time and expertise vary.

## Before you contribute

Before we can use your contributions, you __must__ sign one of the following license agreements. The agreements are self-served at the following links.

Our code review process will automatically check if you have signed the CLA, so
don't fret. Though it may be prudent to check before spending a lot of time on 
contribution.

### Individual Contributions 

For individuals, the [Google Individual
Contributor License Agreement
(CLA)](https://cla.developers.google.com/about/google-individual?csw=1) which is
self served at the link. The CLA is required since you own the copyright to your
changes, even after your contribution becomes part of our codebase, so we need
your permission to use and distribute your code. 

### Corporate Contributions

Contributions made by corporations are covered by the [Google Software Grant and
Corporate Contributor License
Agreement](https://cla.developers.google.com/about/google-corporate).

## Code Reviews

All submissions require a code review via Github Pull Requests.

1. Please adhere to the [Google Style Guide](https://google.github.io/styleguide/cppguide.html) for the language(s) you are submitting in.
2. Keep PRs small and focused. Its good practice and makes it more likely your PR will be approved.
3. Please add tests if possible.
4. Include descriptive commit messages and context to the change/issues fixed.

## Documentation

FlatBuffers uses [MkDocs](https://www.mkdocs.org/) to generate the static
documentation pages served at
[https://flatbuffers.dev](https://flatbuffers.dev). Specifically, we use the
[Material for MkDocs](https://squidfunk.github.io/mkdocs-material/) framework.

The documentation source is contained in the main repo under the
[docs/](https://github.com/google/flatbuffers/tree/master/docs) directory. This
[automatically](https://github.com/google/flatbuffers/blob/46cc3d6432da17cca7694777dcce12e49dd48387/.github/workflows/docs.yml#L6-L11) get built and published when the commit is made.

### Local Development

We encourage contributors to keep the documentation up-to-date as well, and it
is easy to with `MkDocs` local building and serving tools.

First install `mkdocs-material` (see
[Installation](https://squidfunk.github.io/mkdocs-material/getting-started/) for
other ways)

```
pip install mkdocs-material
pip install mkdocs-redirects
```

Then, in the `root` directory of flatbuffers, run 

```
mkdocs serve -f docs/mkdocs.yml
```

This will continually watch the repo for changes to the documentation and serve
the rendered pages locally.

Submit your documentation changes with your code changes and they will
automatically get published when your code is submitted.