################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/algo/neighborhoods/PeerDiscoveryTask.cpp \
../src/algo/neighborhoods/PeerScanner.cpp \
../src/algo/neighborhoods/Aggregate.cpp \
../src/algo/neighborhoods/IPClusterer.cpp \
../src/algo/neighborhoods/TopologyInferrer.cpp

OBJS += \
./src/algo/neighborhoods/PeerDiscoveryTask.o \
./src/algo/neighborhoods/PeerScanner.o \
./src/algo/neighborhoods/Aggregate.o \
./src/algo/neighborhoods/IPClusterer.o \
./src/algo/neighborhoods/TopologyInferrer.o

CPP_DEPS += \
./src/algo/neighborhoods/PeerDiscoveryTask.d \
./src/algo/neighborhoods/PeerScanner.d \
./src/algo/neighborhoods/Aggregate.d \
./src/algo/neighborhoods/IPClusterer.d \
./src/algo/neighborhoods/TopologyInferrer.d

# Each subdirectory must supply rules for building sources it contributes
src/algo/neighborhoods/%.o: ../src/algo/neighborhoods/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -m32 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


