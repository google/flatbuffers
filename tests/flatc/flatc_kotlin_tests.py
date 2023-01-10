# Copyright 2022 Google Inc. All rights reserved.
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

from flatc_test import *


class KotlinTests:

    def EnumValAttributes(self):
        flatc(["--kotlin", "enum_val_attributes.fbs"])

        subject = assert_file_exists("ValAttributes.kt")
        assert_file_doesnt_contains(subject, 'val names : Array<String> = arrayOf("Val1", "Val2", "Val3")')
        assert_file_doesnt_contains(subject, 'fun name(e: Int) : String = names[e]')

    def EnumValAttributes_ReflectNames(self):
        flatc(["--kotlin", "--reflect-names", "enum_val_attributes.fbs"])

        subject = assert_file_exists("ValAttributes.kt")
        assert_file_contains(subject, 'val names : Array<String> = arrayOf("Val1", "Val2", "Val3")')
        assert_file_contains(subject, 'fun name(e: Int) : String = names[e]')
