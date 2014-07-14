; RUN: llc -march=amdil -mcpu=tahiti < %s

%struct._image2d_t = type opaque

@.str = internal addrspace(2) constant [8 x i8] c"dstimg0\00"
@.str1 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str2 = internal addrspace(2) constant [8 x i8] c"dstimg1\00"
@.str3 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str4 = internal addrspace(2) constant [8 x i8] c"dstimg2\00"
@.str5 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str6 = internal addrspace(2) constant [8 x i8] c"dstimg3\00"
@.str7 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str8 = internal addrspace(2) constant [8 x i8] c"dstimg4\00"
@.str9 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str10 = internal addrspace(2) constant [8 x i8] c"dstimg5\00"
@.str11 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str12 = internal addrspace(2) constant [8 x i8] c"dstimg6\00"
@.str13 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str14 = internal addrspace(2) constant [8 x i8] c"dstimg7\00"
@.str15 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str16 = internal addrspace(2) constant [8 x i8] c"dstimg8\00"
@.str17 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str18 = internal addrspace(2) constant [8 x i8] c"dstimg9\00"
@.str19 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str20 = internal addrspace(2) constant [9 x i8] c"dstimg10\00"
@.str21 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str22 = internal addrspace(2) constant [9 x i8] c"dstimg11\00"
@.str23 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str24 = internal addrspace(2) constant [9 x i8] c"dstimg12\00"
@.str25 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str26 = internal addrspace(2) constant [9 x i8] c"dstimg13\00"
@.str27 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str28 = internal addrspace(2) constant [9 x i8] c"dstimg14\00"
@.str29 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str30 = internal addrspace(2) constant [9 x i8] c"dstimg15\00"
@.str31 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str32 = internal addrspace(2) constant [9 x i8] c"dstimg16\00"
@.str33 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str34 = internal addrspace(2) constant [9 x i8] c"dstimg17\00"
@.str35 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str36 = internal addrspace(2) constant [9 x i8] c"dstimg18\00"
@.str37 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str38 = internal addrspace(2) constant [9 x i8] c"dstimg19\00"
@.str39 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str40 = internal addrspace(2) constant [9 x i8] c"dstimg20\00"
@.str41 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str42 = internal addrspace(2) constant [9 x i8] c"dstimg21\00"
@.str43 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str44 = internal addrspace(2) constant [9 x i8] c"dstimg22\00"
@.str45 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str46 = internal addrspace(2) constant [9 x i8] c"dstimg23\00"
@.str47 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str48 = internal addrspace(2) constant [9 x i8] c"dstimg24\00"
@.str49 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str50 = internal addrspace(2) constant [9 x i8] c"dstimg25\00"
@.str51 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str52 = internal addrspace(2) constant [9 x i8] c"dstimg26\00"
@.str53 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str54 = internal addrspace(2) constant [9 x i8] c"dstimg27\00"
@.str55 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str56 = internal addrspace(2) constant [9 x i8] c"dstimg28\00"
@.str57 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str58 = internal addrspace(2) constant [9 x i8] c"dstimg29\00"
@.str59 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str60 = internal addrspace(2) constant [9 x i8] c"dstimg30\00"
@.str61 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str62 = internal addrspace(2) constant [9 x i8] c"dstimg31\00"
@.str63 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str64 = internal addrspace(2) constant [9 x i8] c"dstimg32\00"
@.str65 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str66 = internal addrspace(2) constant [9 x i8] c"dstimg33\00"
@.str67 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str68 = internal addrspace(2) constant [9 x i8] c"dstimg34\00"
@.str69 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str70 = internal addrspace(2) constant [9 x i8] c"dstimg35\00"
@.str71 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str72 = internal addrspace(2) constant [9 x i8] c"dstimg36\00"
@.str73 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str74 = internal addrspace(2) constant [9 x i8] c"dstimg37\00"
@.str75 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str76 = internal addrspace(2) constant [9 x i8] c"dstimg38\00"
@.str77 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str78 = internal addrspace(2) constant [9 x i8] c"dstimg39\00"
@.str79 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str80 = internal addrspace(2) constant [9 x i8] c"dstimg40\00"
@.str81 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str82 = internal addrspace(2) constant [9 x i8] c"dstimg41\00"
@.str83 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str84 = internal addrspace(2) constant [9 x i8] c"dstimg42\00"
@.str85 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str86 = internal addrspace(2) constant [9 x i8] c"dstimg43\00"
@.str87 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str88 = internal addrspace(2) constant [9 x i8] c"dstimg44\00"
@.str89 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str90 = internal addrspace(2) constant [9 x i8] c"dstimg45\00"
@.str91 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str92 = internal addrspace(2) constant [9 x i8] c"dstimg46\00"
@.str93 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str94 = internal addrspace(2) constant [9 x i8] c"dstimg47\00"
@.str95 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str96 = internal addrspace(2) constant [9 x i8] c"dstimg48\00"
@.str97 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str98 = internal addrspace(2) constant [9 x i8] c"dstimg49\00"
@.str99 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str100 = internal addrspace(2) constant [9 x i8] c"dstimg50\00"
@.str101 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str102 = internal addrspace(2) constant [9 x i8] c"dstimg51\00"
@.str103 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str104 = internal addrspace(2) constant [9 x i8] c"dstimg52\00"
@.str105 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str106 = internal addrspace(2) constant [9 x i8] c"dstimg53\00"
@.str107 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str108 = internal addrspace(2) constant [9 x i8] c"dstimg54\00"
@.str109 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str110 = internal addrspace(2) constant [9 x i8] c"dstimg55\00"
@.str111 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str112 = internal addrspace(2) constant [9 x i8] c"dstimg56\00"
@.str113 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str114 = internal addrspace(2) constant [9 x i8] c"dstimg57\00"
@.str115 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str116 = internal addrspace(2) constant [9 x i8] c"dstimg58\00"
@.str117 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str118 = internal addrspace(2) constant [9 x i8] c"dstimg59\00"
@.str119 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str120 = internal addrspace(2) constant [9 x i8] c"dstimg60\00"
@.str121 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str122 = internal addrspace(2) constant [9 x i8] c"dstimg61\00"
@.str123 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str124 = internal addrspace(2) constant [9 x i8] c"dstimg62\00"
@.str125 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@.str126 = internal addrspace(2) constant [9 x i8] c"dstimg63\00"
@.str127 = internal addrspace(2) constant [10 x i8] c"image2d_t\00"
@llvm.image.annotations.__OpenCL_sample_test_kernel = global [64 x <{ i8*, i32 }>] [<{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str2 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str4 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str6 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str8 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str10 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str12 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str14 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str16 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([8 x i8] addrspace(2)* @.str18 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str20 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str22 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str24 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str26 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str28 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str30 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str32 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str34 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str36 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str38 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str40 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str42 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str44 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str46 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str48 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str50 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str52 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str54 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str56 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str58 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str60 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str62 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str64 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str66 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str68 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str70 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str72 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str74 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str76 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str78 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str80 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str82 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str84 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str86 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str88 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str90 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str92 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str94 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str96 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str98 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str100 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str102 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str104 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str106 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str108 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str110 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str112 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str114 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str116 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str118 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str120 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str122 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str124 to i8*), i32 2 }>, <{ i8*, i32 }> <{ i8* bitcast ([9 x i8] addrspace(2)* @.str126 to i8*), i32 2 }>], section "llvm.metadata"
@llvm.argtypename.annotations.__OpenCL_sample_test_kernel = global [64 x i8*] [i8* bitcast ([10 x i8] addrspace(2)* @.str1 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str3 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str5 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str7 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str9 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str11 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str13 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str15 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str17 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str19 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str21 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str23 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str25 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str27 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str29 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str31 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str33 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str35 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str37 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str39 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str41 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str43 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str45 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str47 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str49 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str51 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str53 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str55 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str57 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str59 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str61 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str63 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str65 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str67 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str69 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str71 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str73 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str75 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str77 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str79 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str81 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str83 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str85 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str87 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str89 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str91 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str93 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str95 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str97 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str99 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str101 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str103 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str105 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str107 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str109 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str111 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str113 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str115 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str117 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str119 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str121 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str123 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str125 to i8*), i8* bitcast ([10 x i8] addrspace(2)* @.str127 to i8*)], section "llvm.metadata"
@sgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@fgv = internal addrspace(2) constant [1 x i8] zeroinitializer
@lvgv = internal constant [0 x i8*] zeroinitializer
@rvgv = internal constant [0 x i8*] zeroinitializer
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i8*, i8*, i32 } { i8* bitcast (void (%struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*, %struct._image2d_t addrspace(1)*)* @__OpenCL_sample_test_kernel to i8*), i8* bitcast ([1 x i8] addrspace(2)* @sgv to i8*), i8* bitcast ([1 x i8] addrspace(2)* @fgv to i8*), i8* bitcast ([0 x i8*]* @lvgv to i8*), i8* bitcast ([0 x i8*]* @rvgv to i8*), i32 0 }], section "llvm.metadata"

define void @__OpenCL_sample_test_kernel(%struct._image2d_t addrspace(1)* %dstimg0, %struct._image2d_t addrspace(1)* %dstimg1, %struct._image2d_t addrspace(1)* %dstimg2, %struct._image2d_t addrspace(1)* %dstimg3, %struct._image2d_t addrspace(1)* %dstimg4, %struct._image2d_t addrspace(1)* %dstimg5, %struct._image2d_t addrspace(1)* %dstimg6, %struct._image2d_t addrspace(1)* %dstimg7, %struct._image2d_t addrspace(1)* %dstimg8, %struct._image2d_t addrspace(1)* %dstimg9, %struct._image2d_t addrspace(1)* %dstimg10, %struct._image2d_t addrspace(1)* %dstimg11, %struct._image2d_t addrspace(1)* %dstimg12, %struct._image2d_t addrspace(1)* %dstimg13, %struct._image2d_t addrspace(1)* %dstimg14, %struct._image2d_t addrspace(1)* %dstimg15, %struct._image2d_t addrspace(1)* %dstimg16, %struct._image2d_t addrspace(1)* %dstimg17, %struct._image2d_t addrspace(1)* %dstimg18, %struct._image2d_t addrspace(1)* %dstimg19, %struct._image2d_t addrspace(1)* %dstimg20, %struct._image2d_t addrspace(1)* %dstimg21, %struct._image2d_t addrspace(1)* %dstimg22, %struct._image2d_t addrspace(1)* %dstimg23, %struct._image2d_t addrspace(1)* %dstimg24, %struct._image2d_t addrspace(1)* %dstimg25, %struct._image2d_t addrspace(1)* %dstimg26, %struct._image2d_t addrspace(1)* %dstimg27, %struct._image2d_t addrspace(1)* %dstimg28, %struct._image2d_t addrspace(1)* %dstimg29, %struct._image2d_t addrspace(1)* %dstimg30, %struct._image2d_t addrspace(1)* %dstimg31, %struct._image2d_t addrspace(1)* %dstimg32, %struct._image2d_t addrspace(1)* %dstimg33, %struct._image2d_t addrspace(1)* %dstimg34, %struct._image2d_t addrspace(1)* %dstimg35, %struct._image2d_t addrspace(1)* %dstimg36, %struct._image2d_t addrspace(1)* %dstimg37, %struct._image2d_t addrspace(1)* %dstimg38, %struct._image2d_t addrspace(1)* %dstimg39, %struct._image2d_t addrspace(1)* %dstimg40, %struct._image2d_t addrspace(1)* %dstimg41, %struct._image2d_t addrspace(1)* %dstimg42, %struct._image2d_t addrspace(1)* %dstimg43, %struct._image2d_t addrspace(1)* %dstimg44, %struct._image2d_t addrspace(1)* %dstimg45, %struct._image2d_t addrspace(1)* %dstimg46, %struct._image2d_t addrspace(1)* %dstimg47, %struct._image2d_t addrspace(1)* %dstimg48, %struct._image2d_t addrspace(1)* %dstimg49, %struct._image2d_t addrspace(1)* %dstimg50, %struct._image2d_t addrspace(1)* %dstimg51, %struct._image2d_t addrspace(1)* %dstimg52, %struct._image2d_t addrspace(1)* %dstimg53, %struct._image2d_t addrspace(1)* %dstimg54, %struct._image2d_t addrspace(1)* %dstimg55, %struct._image2d_t addrspace(1)* %dstimg56, %struct._image2d_t addrspace(1)* %dstimg57, %struct._image2d_t addrspace(1)* %dstimg58, %struct._image2d_t addrspace(1)* %dstimg59, %struct._image2d_t addrspace(1)* %dstimg60, %struct._image2d_t addrspace(1)* %dstimg61, %struct._image2d_t addrspace(1)* %dstimg62, %struct._image2d_t addrspace(1)* %dstimg63) nounwind {
entry:
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg0, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg1, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg2, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg3, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg4, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg5, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg6, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg7, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg8, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg9, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg10, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg11, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg12, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg13, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg14, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg15, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg16, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg17, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg18, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg19, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg20, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg21, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg22, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg23, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg24, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg25, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg26, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg27, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg28, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg29, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg30, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg31, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg32, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg33, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg34, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg35, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg36, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg37, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg38, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg39, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg40, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg41, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg42, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg43, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg44, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg45, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg46, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg47, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg48, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg49, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg50, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg51, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg52, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg53, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg54, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg55, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg56, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg57, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg58, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg59, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg60, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg61, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg62, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  tail call void @__amdil_image2d_write(%struct._image2d_t addrspace(1)* %dstimg63, <2 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  ret void
}

declare void @__amdil_image2d_write(%struct._image2d_t addrspace(1)*, <2 x i32>, <4 x i32>) nounwind
