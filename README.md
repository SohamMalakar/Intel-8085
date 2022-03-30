## Intel 8085 Emulator

It's an emulator of the Intel 8085 microprocessor written in C++.

## Build from Source

Run the following command to build the project:

### GNU/Linux

```
$ git clone https://github.com/SohamMalakar/Intel-8085.git
$ cd Intel-8085
$ make
```

## Samples

There are some programs in the [samples](https://github.com/SohamMalakar/Intel-8085/tree/master/samples) directory written in Assembly language. You can run them by executing the following command:

```
$ ./asm samples/test.asm
```

## TODO

I might add these features in the future:

- **next** and **prev** commands to navigate through the memory **(done)**
- step by step execution **(done)**
- breakpoints **(maybe)**
- mnemonics to hexcode conversion
- **bytes**, **m cycles** and **t states** of each instruction **(partially done)**

## License

This project is licensed under the [MIT License](https://github.com/SohamMalakar/Intel-8085/blob/master/LICENSE).

## Contributing

If you want to contribute to the project, please fork the repository and make a [pull request](https://github.com/SohamMalakar/Intel-8085/pulls).

> **_NOTE:_** Recently, I've started to learn the Assembly language. So, it might contain bugs. And I've not implemented all the instructions yet.
