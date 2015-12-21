/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.Runtime.InteropServices;

namespace FlatBuffers
{
    public class Parser
    {
        private IntPtr m_parser;

        public Parser()
        {
            m_parser = flatbuffers_parser_new();
        }

        public bool Parse(string source)
        {
            var ret = flatbuffers_parser_parse(m_parser, source);
            if (ret == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public bool SetRootType(string root_type)
        {
            var result = flatbuffers_parser_set_root_type(m_parser, root_type);
            if (result == 0)
            {
                return true;
            }
            return false;
        }

        public string GenerateJson()
        {
            IntPtr buffer = new IntPtr();
            IntPtr size = new IntPtr();

            flatbuffers_generate_json(m_parser, ref buffer, ref size);
            var result = Marshal.PtrToStringAnsi(buffer);
            if (size.ToInt32() > 0)
            {
                flatbuffers_free_string(buffer);
            }

            return result;
        }

        public byte[] GenerateBuffer()
        {
            IntPtr buffer = new IntPtr();
            IntPtr size = new IntPtr();

            flatbuffers_generate_buffer(m_parser, ref buffer, ref size);

            byte[] managedArray = new byte[size.ToInt32()];
            Marshal.Copy(buffer, managedArray, 0, size.ToInt32());
            if (size.ToInt32() > 0)
            {
                flatbuffers_free_string(buffer);
            }

            return managedArray;
        }

        ~Parser()
        {
            if (m_parser != null)
            {
                flatbuffers_parser_free(m_parser);
            }
        }

        [DllImport("flatbuffers")]
        private static extern IntPtr flatbuffers_parser_new();

        [DllImport("flatbuffers")]
        private static extern void flatbuffers_parser_free(IntPtr parser);

        [DllImport("flatbuffers")]
        private static extern int flatbuffers_parser_parse(IntPtr parser, string source);
    
        [DllImport("flatbuffers")]
        private static extern int flatbuffers_parser_set_root_type(IntPtr parser, string root_type);

        [DllImport("flatbuffers")]
        private static extern int flatbuffers_generate_json(IntPtr parser, ref IntPtr buffer, ref IntPtr size);

        [DllImport("flatbuffers")]
        private static extern int flatbuffers_generate_buffer(IntPtr parser, ref IntPtr buffer, ref IntPtr size);

        [DllImport("flatbuffers")]
        private static extern void flatbuffers_free_string(IntPtr buffer);
    }
}
