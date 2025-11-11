#!/bin/bash
echo "
      ::::::::::: :::     ::::    :::    
         :+:    :+:      :+:+:   :+:     
        +:+   +:+ +:+   :+:+:+  +:+      
       +#+  +#+  +:+   +#+ +:+ +#+       
      +#+ +#+#+#+#+#+ +#+  +#+#+#        
     #+#       #+#   #+#   #+#+#      
###########   ###   ###    ####

=====================================

@2025 Project i4N
"

cmake --build build/Debug --
echo "Build finished"

ELF=$(find build/Debug -name "*.elf" | head -n 1)
if [ ! -f "$ELF" ]; then
    echo "Error: ELF file not found"
    exit 1
fi

echo "Flashing ... "
openocd -f openocd.cfg -c "program $ELF verify reset exit"
echo "Flash finished"
