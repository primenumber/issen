issen
=====

高速なOthelloAIの実装

## ビルド

- C++14に対応したコンパイラと、 Boost C++ Librariesが必要
- Intel C++ Compiler以外でコンパイルできるかどうかは未確認
- SSE\*とpopcountに対応したCPUでないと動作しません

## TODO

- 実際に対戦ができるようにする
- 評価関数をまともなものに変更する
- AVXに移行する
