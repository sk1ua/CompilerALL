# TestCompiler

这是一个简单的类 C 语言编译原理实践项目，包含了一个支持子集语法特性的编译器前端、代码生成器以及一个用于执行中间代码的虚拟机。

## 项目结构

- **`src/`**: 编译器源代码，各文件详细功能如下：
    - **`lex.l`**: Flex 词法分析器定义文件。
        - 负责将源代码字符流转换为 Token（如关键字、标识符、数字、运算符等）。
        - 维护行号和列号信息，用于错误提示。
    - **`lex.h`**: 词法分析器头文件，定义了 Token 枚举和结构体。
    - **`parse.c`**: 递归下降语法分析器实现。
        - 包含 `parse_program`, `parse_statement`, `parse_expression` 等函数。
        - 根据语法规则从 Token 流构建抽象语法树 (AST)。
        - 负责基本的语法错误检测和报告。
    - **`ast.h`**: 抽象语法树 (AST) 数据结构定义。
        - 定义了 `program`, `declaration`, `statement`, `expr` 等核心结构体。
        - 包含枚举类型，如 `type_kind`, `expr_kind` 等，用于区分不同的 AST 节点类型。
    - **`codegen.c`**: 中间代码生成器。
        - 遍历 AST 并生成栈式虚拟机指令（如 `LOAD`, `STO`, `ADD` 等）。
        - 处理函数调用的栈帧管理（`ENTER`, `RETURN`, `CAL`）。
        - 管理符号表和标签生成（用于跳转指令）。
    - **`print.c`**: AST 打印工具。
        - 提供 `print_program` 等函数，将解析后的 AST 以树状结构打印到控制台或文件。
        - 用于调试和验证语法分析的正确性。
    - **`print.h`**: `print.c` 的头文件声明。
    - **`util.c`**: 通用工具函数实现。
        - 包含动态数组 (`vector`) 和 字典树 (`trie`) 的实现。
        - `vector` 用于存储列表数据（如语句列表、参数列表）。
        - `trie` (字典树) 用于符号表管理，快速查找变量名对应的偏移量或信息。
    - **`util.h`**: 工具函数头文件。
    - **`source_position.h`**: 源码位置信息结构定义。
        - 定义 `source_pos` 和 `source_range`，用于在 Token 和 AST 节点中记录源代码位置，方便报错。
    - **`main.c`**: 编译器主程序入口。
        - 协调整个编译过程：调用词法分析 -> 语法分析 -> 打印 AST -> 代码生成。
        - 管理输入输出文件的重定向。
- **`machine.cpp`**: 虚拟机实现，用于执行生成的目标代码
    - 模拟了一个基于栈的计算机。
    - 包含指令解释循环，支持算术运算、逻辑运算、跳转、函数调用等指令。
    - 读取 `code.txt` 并执行。
- **`tests/`**: 测试用例及输入输出文件
    - `input.txt`: 源代码输入。
    - `output.txt`: 编译器输出（AST 结构）。
    - `code.txt`: 编译器生成的中间代码。
- **`CMakeLists.txt`**: CMake 构建配置文件

## 功能特性

- **数据类型**: 支持 `int` 整型
- **控制流**: 支持 `if-else`、`while`、`do-while`、`switch-case`
- **函数**: 支持函数定义与调用（支持递归）
- **输入输出**: 内置 `read` 和 `write` 关键字
- **作用域**: 支持全局变量和局部变量

## 编译构建

本项目使用 CMake 构建，依赖 Flex。

### 前置要求
- C/C++ 编译器 (GCC/Clang/MSVC)
- CMake (3.10+)
- Flex

### 构建步骤

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

构建完成后，将在 `build` 目录下生成两个可执行文件：
- `test_compiler`: 编译器
- `machine`: 虚拟机

## 使用说明

### 1. 编写代码
在 `tests/input.txt` 中编写你的源代码。

示例代码：
```c
int main() {
    int a = 10;
    int b = 20;
    if (a < b) {
        write a;
    } else {
        write b;
    }
    return 0;
}
```

### 2. 运行编译器
在 `build` 目录下运行编译器：

Windows:
```powershell
.\test_compiler.exe
```

Linux/macOS:
```bash
./test_compiler
```

编译器将会：
- 读取 `../tests/input.txt`
- 在 `../tests/output.txt` 中输出抽象语法树 (AST)
- 在 `../tests/code.txt` 中生成中间指令代码

### 3. 运行虚拟机
在 `build` 目录下运行虚拟机来执行生成的代码：

Windows:
```powershell
.\machine.exe
```

Linux/macOS:
```bash
./machine
```

虚拟机将会读取 `../tests/code.txt` 并执行程序，输出结果显示在终端。

## 示例输入

查看 `tests/input.txt` 可以获取更多复杂的测试用例，包括函数调用和循环结构的演示。
