#!/usr/bin/env python
import cv2
File = '20190914_LunaJupiterSaturno/lun000119.pgm'

img = cv2.imread(File)
DEBAYERdict = {
        'BG2BGR':cv2.COLOR_BAYER_BG2BGR,
        'BG2BGRA':cv2.COLOR_BAYER_BG2BGRA,
        'BG2BGR_EA':cv2.COLOR_BAYER_BG2BGR_EA,
        'BG2BGR_VNG':cv2.COLOR_BAYER_BG2BGR_VNG,
        'BG2GRAY':cv2.COLOR_BAYER_BG2GRAY,
        'BG2RGB':cv2.COLOR_BAYER_BG2RGB,
        'BG2RGBA':cv2.COLOR_BAYER_BG2RGBA,
        'BG2RGB_EA':cv2.COLOR_BAYER_BG2RGB_EA,
        'BG2RGB_VNG':cv2.COLOR_BAYER_BG2RGB_VNG,
        'GB2BGR':cv2.COLOR_BAYER_GB2BGR,
        'GB2BGRA':cv2.COLOR_BAYER_GB2BGRA,
        'GB2BGR_EA':cv2.COLOR_BAYER_GB2BGR_EA,
        'GB2BGR_VNG':cv2.COLOR_BAYER_GB2BGR_VNG,
        'GB2GRAY':cv2.COLOR_BAYER_GB2GRAY,
        'GB2RGB':cv2.COLOR_BAYER_GB2RGB,
        'GB2RGB_EA':cv2.COLOR_BAYER_GB2RGB_EA,
        'GR2BGR':cv2.COLOR_BAYER_GR2BGR,
        'GR2BGRA':cv2.COLOR_BAYER_GR2BGRA,
        'GR2BGR_EA':cv2.COLOR_BAYER_GR2BGR_EA,
        'GR2BGR_VNG':cv2.COLOR_BAYER_GR2BGR_VNG,
        'GR2GRAY':cv2.COLOR_BAYER_GR2GRAY,
        'GR2RGB':cv2.COLOR_BAYER_GR2RGB,
        'GR2RGBA':cv2.COLOR_BAYER_GR2RGBA,
        'GR2RGB_EA':cv2.COLOR_BAYER_GR2RGB_EA,
        'GR2RGB_VNG':cv2.COLOR_BAYER_GR2RGB_VNG,
        }

for k in DEBAYERdict:
    dbalg = DEBAYERdict[k]
    colour = cv2.cvtColor(img[300:800,300:800,0],dbalg)
    print('Writing %s'%k)
    cv2.imwrite('rgb_'+k+'.png', colour)


