# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


class OffsetArithmeticError(RuntimeError):
    """
    Error caused by an Offset arithmetic error. Probably caused by bad
    writing of fields. This is considered an unreachable situation in
    normal circumstances.
    """


class NotInObjectError(RuntimeError):
    """
    Error caused by using a Builder to write Object data when not inside
    an Object.
    """


class ObjectIsNestedError(RuntimeError):
    """
    Error caused by using a Builder to begin an Object when an Object is
    already being built.
    """


class StructIsNotInlineError(RuntimeError):
    """
    Error caused by using a Builder to write a Struct at a location that
    is not the current Offset.
    """


class BuilderSizeError(RuntimeError):
    """
    Error caused by causing a Builder to exceed the hardcoded limit of 2
    gigabytes.
    """
