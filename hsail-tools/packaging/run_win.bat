TARGET=%1
mkdir .\build\test\%TARGET%\brig
mkdir .\build\test\%TARGET%\isa
mkdir .\build\test\%TARGET%\bin
.\build\%TARGET%\HSAILasm -assemble -o .\build\test\%TARGET%\brig\BitonicSort_Kernels.brig test\golden\hsail\BitonicSort_Kernels.hsail
.\build\%TARGET%\finalizer .\build\test\%TARGET%\brig\BitonicSort_Kernels.brig .\build\test\%TARGET%\isa\BitonicSort_Kernels.isa .\build\test\%TARGET%\bin\BitonicSort_Kernels.bin
.\build\%TARGET%\HSAILasm -assemble -o .\build\test\%TARGET%\brig\SimpleConvolution_Kernels.brig test\golden\hsail\SimpleConvolution_Kernels.hsail
.\build\%TARGET%\finalizer .\build\test\%TARGET%\brig\SimpleConvolution_Kernels.brig .\build\test\%TARGET%\isa\SimpleConvolution_Kernels.isa .\build\test\%TARGET%\bin\SimpleConvolution_Kernels.bin
.\build\%TARGET%\HSAILasm -assemble -o .\build\test\%TARGET%\brig\FloydWarshall_Kernels.brig test\golden\hsail\FloydWarshall_Kernels.hsail
.\build\%TARGET%\finalizer .\build\test\%TARGET%\brig\FloydWarshall_Kernels.brig .\build\test\%TARGET%\isa\FloydWarshall_Kernels.isa .\build\test\%TARGET%\bin\FloydWarshall_Kernels.bin
.\build\%TARGET%\HSAILasm -assemble -o .\build\test\%TARGET%\brig\FastWalshTransform_Kernels.brig test\golden\hsail\FastWalshTransform_Kernels.hsail
.\build\%TARGET%\finalizer .\build\test\%TARGET%\brig\FastWalshTransform_Kernels.brig .\build\test\%TARGET%\isa\FastWalshTransform_Kernels.isa .\build\test\%TARGET%\bin\FastWalshTransform_Kernels.bin
.\build\%TARGET%\HSAILasm -assemble -o .\build\test\%TARGET%\brig\MatrixMultiplication.brig test\golden\hsail\MatrixMultiplication.hsail
.\build\%TARGET%\finalizer .\build\test\%TARGET%\brig\MatrixMultiplication.brig .\build\test\%TARGET%\isa\MatrixMultiplication.isa .\build\test\%TARGET%\bin\MatrixMultiplication.bin
