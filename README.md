<p align="center">
    <img width="200px" height="auto" src="assets/syringe.png" />
    <h3 align="center">injector</h3>
    <p align="center"><i>This project showcases manual memory allocation in a process without relying on usermode memory APIs</i></p>
</p>

## About
It is composed of three main components:

1. **injector**  
   A kernel driver that communicates with the usermode process through a shared memory page to allocate and map the DLL into the target process.

2. **usermode**  
   A usermode application that interfaces with the kernel driver to facilitate the allocation process.

3. **usermode-lib**  
   A dummy DLL used for testing purposes.

## Support
- Windows 10 x64 22H2
- AMD/Intel


## Usage
1. Ensure that Microsoft [Hyper-V](https://stackoverflow.com/questions/30496116/how-to-disable-hyper-v-in-command-line) is disabled
2. Launch ```usermode.exe [path_to_dll] [target_process]```
3. Sign and load the driver. Otherwise, use [kdmapper](https://github.com/TheCruZ/kdmapper), [KDU](https://github.com/hfiref0x/KDU)
4. Enjoy!

## Credits
- [Frostiest](https://www.unknowncheats.me/forum/anti-cheat-bypass/444289-read-process-physical-memory-attach.html) - R/W physical memory
- @SamuelTulach - README design
