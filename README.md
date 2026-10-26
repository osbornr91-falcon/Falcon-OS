Hello and welcome to Falcon OS!

This README will bring you up to speed on what this project is about and how to best use the project. 

History:
This project is the ongoing work of Reese Osborn, who started this project back in 2021 as a paper OS project Intended to bridge everything that was great about Windows, Mac, and Lunix, while filtering out legacy obligations for a truly modern OS built for the modern user. By late 2025, annoyed by the decisions of Microsoft, Reeese put full effort into finishing the schema diagrams for the OS and began to frame the first modules. Now, in late 2026, he is finally putting the idea into practice and starting the project. 

About:
-Falcon OS was written by Reese Osborn in 2026.
-AI tools were used for debugging, light automation, and analysis for efficency.
-Falcon OS is a custom made Microkernel, and is not based off of any specific OS. No code was taken from any existing OS.
-This is a 64 bit OS
-This OS contains fail safes and safety measures in order to withstand catastrophic losses of hardware capabilities over a short or long period of time while maintinging capabilities to the best of its ability.
-Uses hot swappable modules and systems to facilitate doing signifigant updates without needing to perform restarts and/or terminate service to perform these updates.
-Utilizes unique home grown security architectures to minimize the ability for unwanted programs including malware to execute on the system.
-Contains various operating modes that can be toggled by the user, such as: Turbo Mode, Standard Mode, and Server Mode among others. 
-Utilizes custom file systems to minimize disk thrashing and optimize read and write speeds.
-This OS is NOT designed for mobile devices. These devices include everything that is not a desktop or a dedicated server. Dedicaded servers do not count if they are running on mobile hardware.

Disclaimers:
-The ABOUT section contains mostly planned features. This line will be removed once those features are added.
-This OS may have unintended effects on your mobile devices, I am not liable for unintended effects on said devices as the OS was never designed for those.
-This OS contains no Release ready branch nodes yet. Download and run at your own risk. 

Use:
-Please read LICENSE.md for license information.
-While not mandatory, please credit me should you use the code here to make your own works. The code here is intended to allow authors of other projects to make programs that can be natively run on Falcon OS. 

## Building the kernel prototype

On Linux, install `make`, GCC, binutils, NASM, and `xorriso`. To build the bootable ISO from the repository root:

```sh
make -C falcon_os
```

To boot it in QEMU, install `qemu-system-x86_64` and run:

```sh
make -C falcon_os run
```

The current kernel initializes the Limine framebuffer and CPU/memory topology, draws a white square, and halts. It is an early bootable prototype, not yet a complete operating system. Run `make -C falcon_os clean` to remove generated build outputs.

## PLANNED FEATURES:
-Intelligent automatic on-demand software adapter creation. Will allow windows and mac programs to run on Falcon OS without 3rd party adapter software.
-Smart Antivirus. Designed to render programs innert unless authorized. 
-Various operation modes. Will allow you to directly change the scheduler priority of specific applications during runtime and apply custom rules to govern these changes.
-Offloaded Thinking. Allows you to run 1 or more Falcon OS instance in parallel on different systems to divide and process workloads in parallel
-SAFER_1 AI. Includes a CPU based Deterministic Reasoning Engine called SAFER. This AI will not be automatically installed, and will assist in technical operations and system configurations, among other abilities.
-Cluster mode for SERVER configurations. Allows you to one click add up to 20,000 Falcon OS nodes into a single compute cluster. (depending on hardware avaialable)
-EZY Hardware integration agent. Allows for old or otherwise unknown hardware to be connected via a software adapter
-FLIGHT Browser. Built in default browser for Falcon OS. (no not based on chromium) designed to block ads while providing fast and efficient browsing experience.

And much more. Please put your suggestions for more features in the discussions tab!
