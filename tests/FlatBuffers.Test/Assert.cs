using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FlatBuffers.Test
{

    public class AssertFailedException : Exception
    {
        private readonly object _expected;
        private readonly object _actual;

        public AssertFailedException(object expected, object actual)
        {
            _expected = expected;
            _actual = actual;
        }

        public override string Message
        {
            get { return string.Format("Expected {0} but saw {1}", _expected, _actual); }
        }
    }

    public class AssertUnexpectedThrowException : Exception
    {
        private readonly object _expected;

        public AssertUnexpectedThrowException(object expected)
        {
            _expected = expected;
        }

        public override string Message
        {
            get { return string.Format("Expected exception of type {0}", _expected); }
        }
    }

    public static class Assert
    {
        public static void AreEqual<T>(T expected, T actual)
        {
            if (!expected.Equals(actual))
            {
                throw new AssertFailedException(expected, actual);
            }
        }

        public static void IsTrue(bool value)
        {
            if (!value)
            {
                throw new AssertFailedException(true, value);
            }
        }

        public static void Throws<T>(Action action) where T : Exception
        {
            var caught = false;
            try
            {
                action();
            }
            catch (T ex)
            {
                caught = true;
            }

            if (!caught)
            {
                throw new AssertUnexpectedThrowException(typeof (T));
            }
        }
    }
}
