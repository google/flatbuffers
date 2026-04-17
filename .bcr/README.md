# Bazel Central Registry (BCR) Publishing

This directory contains template files for automated publishing to the [Bazel Central Registry (BCR)](https://github.com/bazelbuild/bazel-central-registry).

## Overview

When a new release tag is created, the GitHub Actions workflow automatically:
1. Generates a BCR entry using these templates
2. Creates attestations for the generated files
3. Opens a pull request against the Bazel Central Registry

## Files

- **metadata.template.json**: Contains repository metadata and maintainer information
- **source.template.json**: Defines the source archive location and format
- **presubmit.yml**: Specifies BCR CI tests to validate the module

## Workflow

The publish workflow is triggered by:
- Release publication (automatic)
- Manual workflow dispatch (for retries or re-publishing)

See `.github/workflows/publish.yaml` and `.github/workflows/release.yml` for the workflow configuration.

## Requirements

The workflow requires a `BCR_PUBLISH_TOKEN` secret to be configured in the repository settings. This should be a GitHub Personal Access Token with:
- `repo` scope
- `workflow` scope

The token should be created by someone with write access to a fork of the bazel-central-registry.

## References

- [Publish to BCR documentation](https://github.com/bazel-contrib/publish-to-bcr)
- [BCR templates](https://github.com/bazel-contrib/publish-to-bcr/tree/main/templates)
- [Bazel Central Registry](https://github.com/bazelbuild/bazel-central-registry)
