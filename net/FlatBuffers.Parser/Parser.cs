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

namespace FlatBuffers.Parser
{
    /// <summary>
    /// provides flatbuffers parser api with P/Invoke.
    /// </summary>
    public class Parser : IDisposable
    {
        [Flags]
        public enum ParserOptions
        {
            DEFAULT = 0,
            ALLOW_NON_UTF8 = 1,
            STRICT_JSON = 2,
            SKIP_UNEXPECTED_FIELDS_IN_JSON = 4,
        }

        private IntPtr m_parser;
        private bool is_disposed = false;

#if UNITY_IPHONE
        // NOTE(chobie): Unity3D's iOS build uses static link, so we use __Internal name here.
        private const string dll_name = "__Internal";
#else
        private const string dll_name = "flatbuffers";
#endif

        public string Error
        {
            get
            {
                IntPtr buffer = new IntPtr();
                IntPtr _size = new IntPtr();

                string result = string.Empty;
                int size = 0;

                flatbuffers_errstr(m_parser, ref buffer, ref _size); ;
                size = _size.ToInt32();
                if (size > 0)
                {
                    byte[] temp = new byte[size];

                    Marshal.Copy(buffer, temp, 0, size);
                    result = System.Text.Encoding.UTF8.GetString(temp);
                }

                return result;
            }
        }


        /// <summary>
        /// create a parser instance.
        /// </summary>
        /// <param name="opt">parser option</param>
        public Parser(ParserOptions opt = 0)
        {
            m_parser = flatbuffers_parser_new((int)opt);
        }

        /// <summary>
        /// parse give string.
        /// 
        /// when you want to parse json and generate buffer. you need 4 steps.
        /// 1) parse flatbuffers scheme.
        /// 2) (optional) call SetRootType
        /// 3) parse json string
        /// 4) call GenerateBuffer api. then you'll get flatbuffers data.
        /// </summary>
        /// <param name="source"></param>
        /// <returns>parse result</returns>
        public bool Parse(string source)
        {
            return flatbuffers_parser_parse(m_parser, source) == 0 ? true : false;
        }

        /// <summary>
        /// set root type.
        /// 
        /// root type requires when parsing json.
        /// </summary>
        /// <param name="root_type">root type name</param>
        /// <returns></returns>
        public bool SetRootType(string root_type)
        {
            return (flatbuffers_parser_set_root_type(m_parser, root_type) == 0) ? true : false;
        }

        /// <summary>
        /// generate json strings
        /// </summary>
        /// <returns>json represented flatbuffers data</returns>
        public string GenerateJson()
        {
            IntPtr buffer = new IntPtr();
            IntPtr _size = new IntPtr();
            string result = string.Empty;

            int size = 0;

            try
            {
                flatbuffers_generate_json(m_parser, ref buffer, ref _size);
                size = _size.ToInt32();
                if (size > 0)
                {
                    byte[] temp = new byte[size];

                    Marshal.Copy(buffer, temp, 0, size);
                    result = System.Text.Encoding.UTF8.GetString(temp);
                }
            }
            finally
            {
                if (size > 0)
                {
                    flatbuffers_free_string(buffer);
                }
            }

            return result;
        }

        /// <summary>
        /// generate flatbuffers data
        /// </summary>
        /// <returns>flatbuffers bytes</returns>
        public byte[] GenerateBuffer()
        {
            IntPtr buffer = new IntPtr();
            IntPtr _size = new IntPtr();
            int size = 0;

            try
            {
                flatbuffers_generate_buffer(m_parser, ref buffer, ref _size);
                size = _size.ToInt32();
                byte[] managedArray = new byte[size];
                Marshal.Copy(buffer, managedArray, 0, size);
                return managedArray;
            }
            finally
            {
                if (size > 0)
                {
                    flatbuffers_free_string(buffer);
                }
            }
        }

        public void Dispose()
        {
            flatbuffers_parser_free(m_parser);
            is_disposed = true;
            GC.SuppressFinalize(this);
        }

        ~Parser()
        {
            Dispose();
        }

        [DllImport(dll_name)]
        private static extern IntPtr flatbuffers_parser_new(int opt);

        [DllImport(dll_name)]
        private static extern void flatbuffers_parser_free(IntPtr parser);

        [DllImport(dll_name)]
        private static extern int flatbuffers_parser_parse(IntPtr parser, string source);

        [DllImport(dll_name)]
        private static extern int flatbuffers_errstr(IntPtr parser, ref IntPtr buffer, ref IntPtr size);

        [DllImport(dll_name)]
        private static extern int flatbuffers_parser_set_root_type(IntPtr parser, string root_type);

        [DllImport(dll_name)]
        private static extern int flatbuffers_generate_json(IntPtr parser, ref IntPtr buffer, ref IntPtr size);

        [DllImport(dll_name)]
        private static extern int flatbuffers_generate_buffer(IntPtr parser, ref IntPtr buffer, ref IntPtr size);

        [DllImport(dll_name)]
        private static extern void flatbuffers_free_string(IntPtr buffer);
    }
}

