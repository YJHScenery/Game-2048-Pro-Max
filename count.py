#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
项目代码行数统计脚本
仅统计C++、QML和UI代码的行数，排除注释和空行
"""

import os
import re
import argparse

# 定义不同文件类型的注释模式
COMMENT_PATTERNS = {
    # C++相关文件
    '.h': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    '.hpp': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    '.cpp': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    '.c': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    '.cc': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    '.cu': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    '.cuh': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    # QML文件
    '.qml': {'single': r'//.*$', 'multi_start': r'/\*', 'multi_end': r'\*/'},
    # UI文件
    '.ui': {'single': r'<!--.*-->', 'multi_start': r'<!--', 'multi_end': r'-->'},
}

# 需要排除的目录
EXCLUDE_DIRS = {
    '.git', '.svn', '.hg', 'CVS', 'cmake-build-debug', 'cmake-build-release',
    'build', 'dist', 'venv', '.venv', '__pycache__', 'node_modules', 'bin', 'obj',
    'Debug', 'Release', 'x64', 'x86', '.idea', '.vscode', "QXlsx", "OpenCV", "include", "lib", "bin", "exprtk", "release"}

# 需要排除的文件
EXCLUDE_FILES = {
    '__init__.py', '.gitignore', '.gitattributes', 'README.md', 'README.txt', "FUTURE.md",
    'LICENSE', 'LICENSE.txt', 'NOTICE', 'NOTICE.txt'
}

# 定义要统计的文件类型分组
FILE_TYPE_GROUPS = {
    'C++': ['.h', '.hpp', '.cpp', '.c', '.cc'],
    'QML': ['.qml'],
    'UI': ['.ui'],
    'CUDA': ['.cuh', '.cu']
}

def get_file_group(ext):
    """
    获取文件类型所属的分组
    :param ext: 文件扩展名
    :return: 文件分组名称，如'C++', 'QML', 'UI'，如果不属于任何分组则返回None
    """
    for group, exts in FILE_TYPE_GROUPS.items():
        if ext in exts:
            return group
    return None

def count_lines(file_path):
    """
    统计单个文件的有效代码行数
    :param file_path: 文件路径
    :return: (总行数, 有效行数)
    """
    _, ext = os.path.splitext(file_path)
    ext = ext.lower()
    
    # 只统计指定类型的文件
    if get_file_group(ext) is None:
        return 0, 0
    
    if ext not in COMMENT_PATTERNS:
        return 0, 0
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return 0, 0
    
    total_lines = len(lines)
    valid_lines = 0
    in_multi_comment = False
    
    patterns = COMMENT_PATTERNS[ext]
    single_comment_pattern = re.compile(patterns['single']) if patterns['single'] else None
    multi_start_pattern = re.compile(patterns['multi_start']) if patterns['multi_start'] else None
    multi_end_pattern = re.compile(patterns['multi_end']) if patterns['multi_end'] else None
    
    for line in lines:
        line = line.strip()
        
        # 空行
        if not line:
            continue
        
        # 处理多行注释
        if multi_start_pattern and multi_end_pattern:
            # 多行注释开始
            if not in_multi_comment:
                if multi_start_pattern.search(line) and not multi_end_pattern.search(line):
                    in_multi_comment = True
                    continue
                elif multi_start_pattern.search(line) and multi_end_pattern.search(line):
                    # 单行多行注释
                    continue
            # 多行注释结束
            else:
                if multi_end_pattern.search(line):
                    in_multi_comment = False
                    # 检查行是否还有有效代码
                    end_pos = multi_end_pattern.search(line).end()
                    if line[end_pos:].strip():
                        valid_lines += 1
                continue
        
        if in_multi_comment:
            continue
        
        # 处理单行注释
        if single_comment_pattern:
            # 检查是否整行都是注释
            if single_comment_pattern.match(line):
                continue
            # 检查行中是否包含注释
            match = single_comment_pattern.search(line)
            if match:
                # 检查注释前是否有有效代码
                if line[:match.start()].strip():
                    valid_lines += 1
                continue
        
        # 有效代码行
        valid_lines += 1
    
    return total_lines, valid_lines

def main():
    parser = argparse.ArgumentParser(description='仅统计C++、QML和UI代码行数')
    parser.add_argument('directory', nargs='?', default='.', help='要统计的目录路径，默认当前目录')
    args = parser.parse_args()
    
    root_dir = args.directory
    if not os.path.isdir(root_dir):
        print(f"错误：{root_dir} 不是有效的目录")
        return
    
    print(f"开始统计目录：{root_dir}")
    print("=" * 60)
    
    # 初始化统计数据
    group_stats = {
        'C++': {'files': 0, 'total': 0, 'valid': 0},
        'QML': {'files': 0, 'total': 0, 'valid': 0},
        'UI': {'files': 0, 'total': 0, 'valid': 0},
        'CUDA': {'files': 0, 'total': 0, 'valid': 0}
    }
    
    # 遍历目录
    for root, dirs, files in os.walk(root_dir):
        # 排除不需要的目录
        print(root, dirs)
        dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]
        
        for file in files:
            # 排除不需要的文件
            if file in EXCLUDE_FILES:
                continue
            
            file_path = os.path.join(root, file)
            _, ext = os.path.splitext(file)
            ext = ext.lower()
            
            total, valid = count_lines(file_path)
            if total == 0 and valid == 0:
                continue
            
            # 获取文件分组
            group = get_file_group(ext)
            if group:
                group_stats[group]['files'] += 1
                group_stats[group]['total'] += total
                group_stats[group]['valid'] += valid
    
    # 计算总计
    total_total = sum(stats['total'] for stats in group_stats.values())
    total_valid = sum(stats['valid'] for stats in group_stats.values())
    total_files = sum(stats['files'] for stats in group_stats.values())
    
    # 输出结果
    print(f"文件类型分组统计:")
    print("=" * 60)
    print(f"{'文件类型':<10} {'文件数':<8} {'总行数':<10} {'有效行数':<10} {'有效行率':<10}")
    print("=" * 60)
    
    # 按有效行数排序输出分组统计
    sorted_groups = sorted(group_stats.items(), key=lambda x: x[1]['valid'], reverse=True)
    
    for group, stats in sorted_groups:
        if stats['files'] > 0:  # 只输出有文件的分组
            valid_ratio = (stats['valid'] / stats['total'] * 100) if stats['total'] > 0 else 0
            print(f"{group:<10} {stats['files']:<8} {stats['total']:<10} {stats['valid']:<10} {valid_ratio:.2f}%")
    
    # 输出总计
    print("=" * 60)
    total_valid_ratio = (total_valid / total_total * 100) if total_total > 0 else 0
    print(f"{'总计':<10} {total_files:<8} {total_total:<10} {total_valid:<10} {total_valid_ratio:.2f}%")
    print("=" * 60)

if __name__ == '__main__':
    main()
    os.system("pause")