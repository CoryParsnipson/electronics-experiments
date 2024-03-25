#!/bin/bash
#
# dump-registers.sh
#
# Reads out the values in the I2C registers (starting from address 0) and
# prints to the screen. Modify $num_regs to match what is in the NUM_REG def
# of the attiny402-i2c-pwm-generator sketch. (Reading past of the end of the
# populated register will return garbage.)

num_regs=5
row_size=16

chip_addr=1
device_addr=0x3F

i2cdetect -y ${chip_addr}

for ADDR in $(seq 1 $num_regs)
do
  ((idx=ADDR-1))

  if !((ADDR)) || ((ADDR % row_size)); then
    echo -n "$(i2cget -y ${chip_addr} ${device_addr} ${idx}) "
  else
    echo $(i2cget -y ${chip_addr} ${device_addr} ${idx})
  fi
done

echo ""
