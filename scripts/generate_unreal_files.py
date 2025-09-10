# Copyright (c) 2025 Golden Hour Games. All Rights Reserved.
# This script is used to generate the Unreal Engine ThirdPartyFlatBuffers module from the vendor flatbuffers directory.

import os
import os.path
import subprocess
import argparse
import sys
import re
from pathlib import Path

FILE_MAPPINGS = {
		'headers_from_include': [
				'allocator.h',
				'array.h',
				'base.h',
				'buffer_ref.h',
				'buffer.h',
				'code_generator.h',
				'code_generators.h',
				'default_allocator.h',
				'detached_buffer.h',
				'file_manager.h',
				'flatbuffer_builder.h',
				'flatbuffers.h',
				'flatc.h',
				'flex_flat_util.h',
				'flexbuffers.h',
				'grpc.h',
				'hash.h',
				'idl.h',
				'minireflect.h',
				'reflection_generated.h',
				'reflection.h',
				'registry.h',
				'stl_emulation.h',
				'string.h',
				'struct.h',
				'table.h',
				'util.h',
				'vector_downward.h',
				'vector.h',
				'verifier.h',
		],
		'headers_from_src': [
				'idl_gen_text.h',
		],
		'pch_headers': [
				('pch/flatc_pch.h', 'pch/flatc_pch.h'),
				('pch/pch.h', 'pch/pch.h'),
		],
		'sources': [
				'idl_gen_text.cpp',
				'idl_parser.cpp',
				'reflection.cpp',
				'util.cpp',
		],
}

FILE_MODIFICATIONS = {
		'base.h': [
				(r'(#ifndef FLATBUFFERS_BASE_H_\n#define FLATBUFFERS_BASE_H_)',
				 r'\1\n\n#if !(UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG || UE_BUILD_SHIPPING || UE_BUILD_TEST)\n#include "ThirdPartyFlatBuffers_Export.h"\n#endif'),
		],
		'idl.h': [
				(r'enum class ProtoIdGapAction \{ NO_OP, WARNING, ERROR \}',
				 r'enum class ProtoIdGapAction { NO_OP, WARNING, P_ERROR }'),
				(r'struct ParserState \{',
				 r'struct THIRDPARTYFLATBUFFERS_API ParserState {'),
				(r'class Parser : public ParserState \{',
				 r'class THIRDPARTYFLATBUFFERS_API Parser : public ParserState {'),

				(r'^extern bool GenerateTextFromTable\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern bool GenerateTextFromTable('),
				(r'^extern const char \*GenerateText\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern const char *GenerateText('),
				(r'^extern const char \*GenerateTextFile\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern const char *GenerateTextFile('),
				(r'^extern const char \*GenTextFromTable\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern const char *GenTextFromTable('),
				(r'^extern const char \*GenText\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern const char *GenText('),
				(r'^extern const char \*GenTextFile\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern const char *GenTextFile('),
				(r'^extern bool GenerateSwiftGRPC\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern bool GenerateSwiftGRPC('),
				(r'^extern bool GenerateTSGRPC\(',
				 r'THIRDPARTYFLATBUFFERS_API\nextern bool GenerateTSGRPC('),
		],
		'util.h': [
				(r'^bool FileExists\(const char \*name\);',
				 r'THIRDPARTYFLATBUFFERS_API bool FileExists(const char *name);'),
				(r'^bool DirExists\(const char \*name\);',
				 r'THIRDPARTYFLATBUFFERS_API bool DirExists(const char *name);'),
				(r'^bool LoadFile\(const char \*name, bool binary, std::string \*buf\);',
				 r'THIRDPARTYFLATBUFFERS_API bool LoadFile(const char *name, bool binary, std::string *buf);'),
				(r'^bool SaveFile\(const char \*name, const char \*buf, size_t len, bool binary\);',
				 r'THIRDPARTYFLATBUFFERS_API bool SaveFile(const char *name, const char *buf, size_t len, bool binary);'),
		],
}


def get_default_paths():
		script_dir = Path(__file__).parent
		if 'Vendor' in str(script_dir):
				vendor_dir = script_dir.parent
				project_root = vendor_dir.parent.parent
		elif 'Tools' in str(script_dir):
				project_root = script_dir.parent
				vendor_dir = project_root / 'Vendor' / 'flatbuffers'
		else:
				vendor_dir = Path('..') / 'Vendor' / 'flatbuffers'
				project_root = Path('..')

		module_dir = project_root / 'DawnUE' / \
				'Source' / 'Game' / 'ThirdPartyFlatBuffers'

		return vendor_dir, module_dir


def ensure_directory_exists(directory):
		if not os.path.exists(directory):
				os.makedirs(directory)


def apply_modifications_to_content(content, filename):
		"""Apply modifications to file content if needed"""
		if filename not in FILE_MODIFICATIONS:
				return content, False

		original_content = content
		for pattern, replacement in FILE_MODIFICATIONS[filename]:
				content = re.sub(pattern, replacement, content, flags=re.MULTILINE)

		return content, (content != original_content)


def copy_and_modify_file(source_file, dest_file, use_p4=True, verbose=False):
		"""Copy file and apply modifications in a single operation"""
		filename = os.path.basename(dest_file)

		with open(source_file, 'r', encoding='utf-8') as f:
				content = f.read()

		content, was_modified = apply_modifications_to_content(content, filename)

		file_exists = os.path.isfile(dest_file)

		if file_exists:
				with open(dest_file, 'r', encoding='utf-8') as f:
						existing_content = f.read()

				if existing_content == content:
						if verbose:
								print(f'No change detected for {filename}')
						return False

				print(f'Change detected: {filename}')
				if use_p4:
						result = subprocess.run(
								['p4', 'edit', str(dest_file)], capture_output=True, text=True)
						if result.returncode != 0 and 'already opened for edit' not in result.stderr:
								print(
										f'  Warning: Could not check out file from Perforce: {result.stderr}')
		else:
				print(f'New file detected: {filename}')

		with open(dest_file, 'w', encoding='utf-8') as f:
				f.write(content)

		if not file_exists and use_p4:
				subprocess.run(['p4', 'add', str(dest_file)])

		if verbose and was_modified:
				print(f'  Applied Unreal modifications to {filename}')

		return True


def process_header_files(vendor_dir, module_dir, use_p4, verbose):
		changes_made = False

		vendor_include_dir = vendor_dir / 'include' / 'flatbuffers'
		vendor_src_dir = vendor_dir / 'src'
		module_public_dir = module_dir / 'Public' / 'flatbuffers'

		ensure_directory_exists(module_public_dir)

		print('Processing header files from include directory...')
		for header_file in FILE_MAPPINGS['headers_from_include']:
				source_file = vendor_include_dir / header_file
				dest_file = module_public_dir / header_file

				if not source_file.exists():
						print(f'WARNING: Source file not found: {source_file}')
						continue

				if copy_and_modify_file(source_file, dest_file, use_p4, verbose):
						changes_made = True

		print('Processing header files from src directory...')
		for header_file in FILE_MAPPINGS['headers_from_src']:
				source_file = vendor_src_dir / header_file
				dest_file = module_public_dir / header_file

				if not source_file.exists():
						print(f'WARNING: Source file not found: {source_file}')
						continue

				if copy_and_modify_file(source_file, dest_file, use_p4, verbose):
						changes_made = True

		module_pch_dir = module_public_dir / 'pch'
		ensure_directory_exists(module_pch_dir)
		print('Processing header files from pch directory...')
		for source_path, dest_path in FILE_MAPPINGS['pch_headers']:
				source_file = vendor_include_dir / source_path
				dest_file = module_public_dir / dest_path

				if not source_file.exists():
						print(f'WARNING: PCH source file not found: {source_file}')
						continue

				if copy_and_modify_file(source_file, dest_file, use_p4, verbose):
						changes_made = True

		return changes_made


def process_source_files(vendor_dir, module_dir, use_p4, verbose):
		changes_made = False
		vendor_src_dir = vendor_dir / 'src'
		module_private_dir = module_dir / 'Private'
		ensure_directory_exists(module_private_dir)

		print('Processing source files...')
		for source_file_name in FILE_MAPPINGS['sources']:
				source_file = vendor_src_dir / source_file_name
				dest_file = module_private_dir / source_file_name

				if not source_file.exists():
						print(f'WARNING: Source file not found: {source_file}')
						continue

				if source_file_name == 'idl_gen_text.cpp':
						with open(source_file, 'r', encoding='utf-8') as f:
								content = f.read()

						content = content.replace(
								'#include "idl_gen_text.h"', '#include "flatbuffers/idl_gen_text.h"')

						file_exists = os.path.isfile(dest_file)

						if file_exists:
								with open(dest_file, 'r', encoding='utf-8') as f:
										existing_content = f.read()

								if existing_content == content:
										if verbose:
												print(f'No change detected for {source_file_name}')
										continue

								print(f'Change detected: {source_file_name}')
								if use_p4:
										subprocess.run(['p4', 'edit', str(dest_file)])
						else:
								print(f'New file detected: {source_file_name}')

						with open(dest_file, 'w', encoding='utf-8') as f:
								f.write(content)

						if not file_exists and use_p4:
								subprocess.run(['p4', 'add', str(dest_file)])

						if verbose:
								print(f'  Fixed include path in idl_gen_text.cpp')

						changes_made = True
				else:
						if copy_and_modify_file(source_file, dest_file, use_p4, verbose):
								changes_made = True

		return changes_made


def main():
		parser = argparse.ArgumentParser(
				description='Copy FlatBuffers files from vendor directory to Unreal Engine ThirdParty module.'
		)

		parser.add_argument(
				'-v', '--verbose',
				action='store_true',
				help='Increase output verbosity'
		)

		parser.add_argument(
				'--no-p4',
				action='store_true',
				help='Skip perforce operations'
		)

		parser.add_argument(
				'--vendor-dir',
				type=str,
				help='Path to the vendor flatbuffers directory (default: auto-detect)'
		)

		parser.add_argument(
				'--module-dir',
				type=str,
				help='Path to the ThirdPartyFlatBuffers module directory (default: auto-detect)'
		)

		args = parser.parse_args()
		default_vendor_dir, default_module_dir = get_default_paths()
		vendor_dir = Path(
				args.vendor_dir) if args.vendor_dir else default_vendor_dir
		module_dir = Path(
				args.module_dir) if args.module_dir else default_module_dir

		if not vendor_dir.exists():
				print(f'ERROR: Vendor directory not found: {vendor_dir}')
				print('Please specify the correct path with --vendor-dir')
				return 1

		if not module_dir.exists():
				print(f'ERROR: Module directory not found: {module_dir}')
				return 1

		print(f'Vendor directory: {vendor_dir}')
		print(f'Module directory: {module_dir}')

		use_p4 = not args.no_p4

		changes_made = False
		if process_header_files(vendor_dir, module_dir, use_p4, args.verbose):
				changes_made = True
		if process_source_files(vendor_dir, module_dir, use_p4, args.verbose):
				changes_made = True

		if changes_made:
				print('\nFiles updated successfully!')
		else:
				print('\nNo changes detected - all files are up to date')

		return 0


if __name__ == '__main__':
		sys.exit(main())
