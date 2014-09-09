using System;
using System.Linq;
using System.Reflection;

namespace FlatBuffers.Test
{
    static class Program
    {
        public static int Main(string[] args)
        {
            var tests = new FlatBuffersExampleTests();
            try
            {
                tests.RunTests();
            }
            catch (Exception ex)
            {
                Console.WriteLine("FlatBuffersExampleTests FAILED - {0}", ex.GetBaseException());
                return -1;
            }

            // Run ByteBuffers Tests
            var testClass = new ByteBufferTests();

            var methods = testClass.GetType().GetMethods(BindingFlags.Public | BindingFlags.Instance).Where(m => m.Name.StartsWith("ByteBuffer_"));
            foreach (var method in methods)
            {
                try
                {
                    method.Invoke(testClass, new object[] { });
                }
                catch (Exception ex)
                {
                    Console.WriteLine("ByteBufferTests FAILED when invoking {0} with error {1}", method.Name, ex.GetBaseException());
                    return -1;
                }
                
            }

            return 0;
        }
    }
}
