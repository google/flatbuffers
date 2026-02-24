// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using BenchmarkDotNet.Environments;
using BenchmarkDotNet.Exporters;
using BenchmarkDotNet.Loggers;
using BenchmarkDotNet.Reports;
using BenchmarkDotNet.Running;

namespace FlatSpanBuffers.Benchmarks;

public static class Program
{
    public static void Main(string[] args)
    {
        List<Summary> results =
        [
            BenchmarkRunner.Run<SimpleMonsterBenchmarks>(),
            BenchmarkRunner.Run<DecodeBenchmarks>(),
            BenchmarkRunner.Run<DecodeObjectApiBenchmarks>(),
            BenchmarkRunner.Run<EncodeBenchmarks>(),
            BenchmarkRunner.Run<EncodeObjectApiBenchmarks>(),
            BenchmarkRunner.Run<VerifyBenchmarks>(),
        ];

        if (results.Count == 0)
            return;

        var logger = ConsoleLogger.Default;

        Console.WriteLine();
        logger.WriteLine(LogKind.Header, @"// * Final Summary *");
        Console.WriteLine();

        logger.WriteLine(LogKind.Default, "```");
        logger.WriteLine(LogKind.Info, HostEnvironmentInfo.GetInformation());
        foreach (var summary in results)
        {
            logger.WriteLine(LogKind.Header, $"{summary.Title}");
            PrintTableViaReflection(summary, logger, MarkdownExporter.Console);
            Console.WriteLine();
        }
        logger.WriteLine(LogKind.Default, "```");
    }

    private static void PrintTableViaReflection(Summary summary, ILogger logger, IExporter exporter)
    {
        var exporterType = exporter.GetType();
        var printTableMethod = exporterType.GetMethod(
            "PrintTable",
            BindingFlags.NonPublic | BindingFlags.Instance, null,
            new[] { typeof(SummaryTable), typeof(ILogger) }, null);

        if (printTableMethod == null)
        {
            logger.WriteLine(LogKind.Error, "Could not find PrintTable method via reflection");
        }

        printTableMethod.Invoke(exporter, new object[] { summary.Table, logger });
    }
}
