clear;
clc;

a_32(1:256) = uint32(0);
a_32(1)     = uint32(65537);

for i=2:256
    if(a_32(i-1) == uint32(2147516416))
        a_32(i) = uint32(65537);
    else
        a_32(i) = uint32(a_32(i-1)*2);
    end
end

a_32 = a_32';

b_32 = dec2hex(a_32, 8);