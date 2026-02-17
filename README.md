# CCVII - Assignments
Collection of assignments for the Operating Systems course.  

## Compiling and running
Use `make PROJECT=project_directory TARGET=target_device` to compile the given project __project_directory__.  
  
The parameter `PROJECT` only needs to be given once, as it is saved in the `.config.mk` configuration file. Specify it only when the target project is being changed.  

The parameter `TARGET` is meant to specify either the project is to be run on QEMU emulation or on BeagleBone Black minicomputer. Choose between `TARGET=qemu`(default) and `TARGET=beagle`.  
  
To run the project, use `make run PROJECT=project_directory TARGET=target_device` (the `PROJECT` parameter works same as detailed before). For the moment, running is only supported for QEMU as target.  

To debug, either use `make debug PROJECT=project_directory` or use VSCode's native debugging with the [Cortex-Debug extension](https://github.com/Marus/cortex-debug.git). Take in consideration that debugging is only supported with QEMU as the target.  

Consult `make help` for details.

## Projects
- [001HelloWorld_Qemu]()
- [002Calculadora](002Calculadora/Documentation.md)
- [003InterruptHandler](003InterruptHandler/Documentation.md)
- [004Process](004Process/Documentation.md)