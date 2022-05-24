#!/bin/bash
cpuMode=$1;

function cpuparams() {
    echo "start print cpu params."
##############################find core cluster#########################################
all_max_freqs=$(adb shell cat "/sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq")
declare -a freqs=($all_max_freqs)
cores=${#freqs[@]}
super=$cores
little=0
for ((i = 1; i < $cores; i++)); do
	if [[ ${freqs[$i]} > ${freqs[$i - 1]} ]]; then
		if [ -z "$big" ]; then
			big=$i
		else
			super=$i
			break
		fi
	elif [[ ${freqs[$i]} < ${freqs[$i - 1]} ]]; then
		big=0;
		little=$i
	fi
done



##############################硬件限制cpufreq#################################################
hard_min_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/cpuinfo_min_freq; echo \$line")
hard_min_little=$(($hard_min_little/1000))
hard_max_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/cpuinfo_max_freq; echo \$line")
hard_max_little=$(($hard_max_little/1000))

hard_min_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/cpuinfo_min_freq; echo \$line")
hard_min_big=$(($hard_min_big/1000))
hard_max_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/cpuinfo_max_freq; echo \$line")
hard_max_big=$(($hard_max_big/1000))

if [  $super -lt $cores ]; then
hard_min_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/cpuinfo_min_freq; echo \$line")
hard_min_super=$(($hard_min_super/1000))
hard_max_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/cpuinfo_max_freq; echo \$line")
hard_max_super=$(($hard_max_super/1000))
fi

#---------------------------------------------------------------------

thermal_files_cmd="
count=\`ls -d /sys/class/thermal/thermal_zone* | wc -w\`
let count--
for i in \`seq 0 \$count\`
do
	dir=/sys/class/thermal/thermal_zone\$i
	read device < \$dir/type
	if [   \$device = xo_therm \
		-o \$device = quiet_therm \
		-o \$device = battery \
		-o \$device = cpuss-0-usr \
		-o \$device = cpuss-1-usr \
		-o \$device = camera-usr \
		]; then
		echo \$device
		echo \$dir/temp
	fi
done
"


####################################################################
#******************************************************************#
####################################################################

while true
do

######################### pid calc##########################################################
ps_all=`adb shell ps -ef`

cam_app_pid=$(grep "com.android.camera" <<< "$ps_all" | awk '{print $2}')
cam_server_pid=$(grep "cameraserver$" <<< "$ps_all" | awk '{print $2}')
cam_provider_pid=$(grep "camera.provider" <<< "$ps_all" | awk '{print $2}')



#################### CPU freq governor ##########################################
little_governor=$(adb shell cat "/sys/devices/system/cpu/cpu${little}/cpufreq/scaling_governor")
little_up_rate_limit_us=$(adb shell cat "/sys/devices/system/cpu/cpu${little}/cpufreq/schedutil/up_rate_limit_us")
little_down_rate_limit_us=$(adb shell cat "/sys/devices/system/cpu/cpu${little}/cpufreq/schedutil/down_rate_limit_us")
little_pl=$(adb shell cat "/sys/devices/system/cpu/cpu${little}/cpufreq/schedutil/pl")
little_hispeed_load=$(adb shell cat "/sys/devices/system/cpu/cpu${little}/cpufreq/schedutil/hispeed_load")
little_hispeed_freq=$(adb shell cat "/sys/devices/system/cpu/cpu${little}/cpufreq/schedutil/hispeed_freq")

big_governor=$(adb shell cat "/sys/devices/system/cpu/cpu${big}/cpufreq/scaling_governor")
big_up_rate_limit_us=$(adb shell cat "/sys/devices/system/cpu/cpu${big}/cpufreq/schedutil/up_rate_limit_us")
big_down_rate_limit_us=$(adb shell cat "/sys/devices/system/cpu/cpu${big}/cpufreq/schedutil/down_rate_limit_us")
big_pl=$(adb shell cat "/sys/devices/system/cpu/cpu${big}/cpufreq/schedutil/pl")
big_hispeed_freq=$(adb shell cat "/sys/devices/system/cpu/cpu${big}/cpufreq/schedutil/hispeed_freq")
big_hispeed_load=$(adb shell cat "/sys/devices/system/cpu/cpu${big}/cpufreq/schedutil/hispeed_load")


if [  $super -lt $cores ]; then
super_governor=$(adb shell cat "/sys/devices/system/cpu/cpu${super}/cpufreq/scaling_governor")
super_up_rate_limit_us=$(adb shell cat "/sys/devices/system/cpu/cpu${super}/cpufreq/schedutil/up_rate_limit_us")
super_down_rate_limit_us=$(adb shell cat "/sys/devices/system/cpu/cpu${super}/cpufreq/schedutil/down_rate_limit_us")
super_pl=$(adb shell cat "/sys/devices/system/cpu/cpu${super}/cpufreq/schedutil/pl")
super_hispeed_load=$(adb shell cat "/sys/devices/system/cpu/cpu${super}/cpufreq/schedutil/hispeed_load")
super_hispeed_freq=$(adb shell cat "/sys/devices/system/cpu/cpu${super}/cpufreq/schedutil/hispeed_freq")
fi
##################### CPU cgroup param ##########################################
#schedtune
#rt_prefer_idle=$(adb shell cat "/dev/stune/rt/schedtune.prefer_idle")
#rt_boost=$(adb shell cat "/dev/stune/rt/schedtune.boost")
#top_prefer_idle=$(adb shell cat "/dev/stune/top-app/schedtune.prefer_idle")
#top_boost=$(adb shell cat "/dev/stune/top-app/schedtune.boost")
#foreground_prefer_idle=$(adb shell cat "/dev/stune/foreground/schedtune.prefer_idle")
#foreground_boost=$(adb shell cat "/dev/stune/foreground/schedtune.boost")
#background_prefer_idle=$(adb shell cat "/dev/stune/background/schedtune.prefer_idle")
#background_boost=$(adb shell cat "/dev/stune/background/schedtune.boost")
#sys_background_prefer_idle=$(adb shell cat "/dev/stune/system-background/schedtune.prefer_idle")
#sys_background_boost=$(adb shell cat "/dev/stune/system-background/schedtune.boost")

#cpuset
top_cpuset=$(adb shell cat "/dev/cpuset/top-app/cpus")
foreground_cpuset=$(adb shell cat "/dev/cpuset/foreground/cpus")
background_cpuset=$(adb shell cat "/dev/cpuset/background/cpus")
sys_background_cpuset=$(adb shell cat "/dev/cpuset/system-background/cpus")

#################################################################################

date

echo "==============================================================="
echo "==============================================================="
printf "CPUFreq schedutil info dump           %20s\n" "i:$((k++))"

printf "            governor  up_rate_limit_us  down_rate_limit_us  hispeed_load  hispeed_freq      pl\n"
printf "小核%d-%d ：  %s       %3d               %4d                %2d           %2d       %d \n" \
        $little $((big - 1))  \
        $little_governor  $little_up_rate_limit_us $little_down_rate_limit_us  \
        $little_hispeed_load $little_hispeed_freq $little_pl

printf "大核%d-%d ：  %s       %3d               %4d                %2d           %2d       %d \n" \
        $big    $((super - 1)) $big_governor \
        $big_up_rate_limit_us $big_down_rate_limit_us  \
        $big_hispeed_load $big_hispeed_freq $big_pl

if [  $super -lt $cores ]; then
printf "超核%d-%d ：  %s       %3d               %4d                %2d           %2d       %d\n" \ 
        $super  $((cores - 1)) \
        $super_governor  $super_up_rate_limit_us $super_down_rate_limit_us \
        $super_hispeed_load $super_hispeed_freq $super_pl
fi




echo "======================================================="
printf "cpuset:          %4s           %4s              %3s              %3s\n"     \
        $top_cpuset  $foreground_cpuset   $background_cpuset   $sys_background_cpuset 


echo "-----------------------------------------------------------"

printf "%-18s %5s ℃\n"  "Device"       "Temp"
printf "%-18s %5.1f\n"   "camera:"      $temp_camera
printf "%-18s %5.1f\n"   "board_hot:"   $temp_xo_therm
printf "%-18s %5.1f\n"   "board_cold:"  $temp_quiet_therm
printf "%-18s %5.1f\n"   "battery:"     $temp_battery

#printf "%-18s %5.1f\n"   "F10-camera:"     $(echo "$temp_quiet_therm*3/4 + $temp_battery/4" | bc)

printf "\n"


##################### while END #########################

done

}

#!/bin/bash

function cpudump() {
    echo "start print cpu dump."
all_max_freqs=$(adb shell cat "/sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq")
declare -a freqs=($all_max_freqs)
cores=${#freqs[@]}
super=$cores
little=0
for ((i = 1; i < $cores; i++)); do
	if [[ ${freqs[$i]} > ${freqs[$i - 1]} ]]; then
		if [ -z "$big" ]; then
			big=$i
		else
			super=$i
			break
		fi
	elif [[ ${freqs[$i]} < ${freqs[$i - 1]} ]]; then
		big=0;
		little=$i
	fi
done

hard_min_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/cpuinfo_min_freq; echo \$line")
hard_min_little=$(($hard_min_little/1000))
hard_max_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/cpuinfo_max_freq; echo \$line")
hard_max_little=$(($hard_max_little/1000))

hard_min_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/cpuinfo_min_freq; echo \$line")
hard_min_big=$(($hard_min_big/1000))
hard_max_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/cpuinfo_max_freq; echo \$line")
hard_max_big=$(($hard_max_big/1000))

if [  $super -lt $cores ]; then
hard_min_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/cpuinfo_min_freq; echo \$line")
hard_min_super=$(($hard_min_super/1000))
hard_max_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/cpuinfo_max_freq; echo \$line")
hard_max_super=$(($hard_max_super/1000))
fi

#---------------------------------------------------------------------

thermal_files_cmd="
count=\`ls -d /sys/class/thermal/thermal_zone* | wc -w\`
let count--
for i in \`seq 0 \$count\`
do
	dir=/sys/class/thermal/thermal_zone\$i
	read device < \$dir/type
	if [   \$device = xo_therm \
		-o \$device = quiet_therm \
		-o \$device = battery \
		-o \$device = cpuss-0-usr \
		-o \$device = cpuss-1-usr \
		-o \$device = camera-usr \
		]; then
		echo \$device
		echo \$dir/temp
	fi
done
"
thermal_files=($(adb shell "$thermal_files_cmd"))

for((i=0; i < ${#thermal_files[*]}; i++))
do
	device=${thermal_files[$i]}
	let i++
	if [ $device = cpuss-0-usr ]; then
		temp_file_little=${thermal_files[$i]}
	elif [ $device = cpuss-1-usr ]; then
		temp_file_big=${thermal_files[$i]}
	elif [ $device = camera-usr ]; then
		temp_file_camera=${thermal_files[$i]}
	elif [ $device = xo_therm ]; then
		temp_file_xo_therm=${thermal_files[$i]}
	elif [ $device = quiet_therm ]; then
		temp_file_quiet_therm=${thermal_files[$i]}
	elif [ $device = battery ]; then
		temp_file_battery=${thermal_files[$i]}
	else
		echo "Error, pls check the code"
	fi
done


####################################################################
#******************************************************************#
####################################################################

while true
do

#################### while START ###################### 

perf_min_all=$(adb shell "read line < /sys/module/msm_performance/parameters/cpu_min_freq; echo \$line")
perf_max_all=$(adb shell "read line < /sys/module/msm_performance/parameters/cpu_max_freq; echo \$line")

perf_min_little=$(awk -F"[ :]" "{print \$$(( (little + 1)*2 ))}" <<< "$perf_min_all")
perf_min_little=$(($perf_min_little/1000))

perf_max_little=$(awk -F"[ :]" "{print \$$(( (little + 1)*2 ))}" <<< "$perf_max_all")
perf_max_little=$(($perf_max_little/1000))
perf_max_little=$(($perf_max_little < 9999 ? $perf_max_little : 9999))

perf_min_big=$(awk -F"[ :]" "{print \$$(( (big + 1)*2 ))}" <<< "$perf_min_all")
perf_min_big=$(($perf_min_big/1000))

perf_max_big=$(awk -F"[ :]" "{print \$$(( (big + 1)*2 ))}" <<< "$perf_max_all")
perf_max_big=$(($perf_max_big/1000))
perf_max_big=$(($perf_max_big < 9999 ? $perf_max_big : 9999))

if [  $super -lt $cores ]; then
perf_min_super=$(awk -F"[ :]" "{print \$$(( (super + 1)*2 ))}" <<< "$perf_min_all")
perf_min_super=$(($perf_min_super/1000))

perf_max_super=$(awk -F"[ :]" "{print \$$(( (super + 1)*2 ))}" <<< "$perf_max_all")
perf_max_super=$(($perf_max_super/1000))
perf_max_super=$(($perf_max_super < 9999 ? $perf_max_super : 9999))
fi

#---------------------------------------------------------------------

user_min_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/scaling_min_freq; echo \$line")
user_min_little=$(($user_min_little/1000))

user_max_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/scaling_max_freq; echo \$line")
user_max_little=$(($user_max_little/1000))

user_min_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/scaling_min_freq; echo \$line")
user_min_big=$(($user_min_big/1000))

user_max_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/scaling_max_freq; echo \$line")
user_max_big=$(($user_max_big/1000))

if [  $super -lt $cores ]; then
user_min_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/scaling_min_freq; echo \$line")
user_min_super=$(($user_min_super/1000))

user_max_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/scaling_max_freq; echo \$line")
user_max_super=$(($user_max_super/1000))
fi

#---------------------------------------------------------------------

cur_freq_little=$(adb shell "read line < /sys/devices/system/cpu/cpu${little}/cpufreq/cpuinfo_cur_freq; echo \$line")
cur_freq_little=$(($cur_freq_little/1000))

cur_freq_big=$(adb shell "read line < /sys/devices/system/cpu/cpu${big}/cpufreq/cpuinfo_cur_freq; echo \$line")
cur_freq_big=$(($cur_freq_big/1000))

if [  $super -lt $cores ]; then
cur_freq_super=$(adb shell "read line < /sys/devices/system/cpu/cpu${super}/cpufreq/cpuinfo_cur_freq; echo \$line")
	if [ $cur_freq_super = "<unknown>" ] ;then
		cur_freq_super=0
	fi
cur_freq_super=$(($cur_freq_super/1000))
fi

#---------------------------------------------------------------------

temp_little=$(adb shell "read line < $temp_file_little; echo \$line")
temp_little=$(echo "scale=1;$temp_little/1000" | bc)

temp_big=$(adb shell "read line < $temp_file_big; echo \$line")
temp_big=$(echo "scale=1;$temp_big/1000" | bc)

temp_camera=$(adb shell "read line < $temp_file_camera; echo \$line")
temp_camera=$(echo "scale=1;$temp_camera/1000" | bc)

temp_xo_therm=$(adb shell "read line < $temp_file_xo_therm; echo \$line")
temp_xo_therm=$(echo "scale=1;$temp_xo_therm/1000" | bc)

temp_quiet_therm=$(adb shell "read line < $temp_file_quiet_therm; echo \$line")
temp_quiet_therm=$(echo "scale=1;$temp_quiet_therm/1000" | bc)

temp_battery=$(adb shell "read line < $temp_file_battery; echo \$line")
temp_battery=$(echo "scale=1;$temp_battery/1000" | bc)


#---------------------------------------------------------------------

onlines=$(adb shell "cat /sys/devices/system/cpu/cpu*/online")
onlines=$(echo $onlines)
isolates=$(adb shell "cat /sys/devices/system/cpu/cpu*/isolate")
isolates=$(echo $isolates)


###################################################################################


date

echo "============================================================"
printf "CPUFreq and Thermal info dump           %20s\n" "i:$((k++))"

order=$(eval echo {0..$((cores - 1))})
printf "cores   : %s\n"  "$order"
printf "online  : %s\n" "$onlines"
printf "isolate : %s\n" "$isolates"

echo "-----------------------------------------------------------"

printf "         freq_range    perf_limit   final_limit  freq  temp \n"

printf "小核%d-%d (%3d--%4d):  [%4d--%4d] [%4d--%4d]  %4d  %3.1f\n" \
        $little $((big   - 1)) \
        $hard_min_little $hard_max_little \
        $perf_min_little $perf_max_little \
        $user_min_little $user_max_little \
        $cur_freq_little \
        $temp_little

printf "大核%d-%d (%3d--%4d):  [%4d--%4d] [%4d--%4d]  %4d  %3.1f\n" \
        $big    $((super - 1)) \
        $hard_min_big $hard_max_big \
        $perf_min_big $perf_max_big \
        $user_min_big $user_max_big \
        $cur_freq_big \
        $temp_big

if [  $super -lt $cores ]; then
printf "超核%d-%d (%3d--%4d):  [%4d--%4d] [%4d--%4d]  %4d  %3.1f\n" \
        $super  $((cores - 1)) \
        $hard_min_super $hard_max_super \
        $perf_min_super $perf_max_super \
        $user_min_super $user_max_super \
        $cur_freq_super \
        $temp_big
fi

echo "-----------------------------------------------------------"

printf "%-18s %5s ℃\n"  "Device"       "Temp"
printf "%-18s %5.1f\n"   "camera:"      $temp_camera
printf "%-18s %5.1f\n"   "board_hot:"   $temp_xo_therm
printf "%-18s %5.1f\n"   "board_cold:"  $temp_quiet_therm
printf "%-18s %5.1f\n"   "battery:"     $temp_battery

printf "\n"


##################### while END #########################

done
}

adb root;
#if [ $cpuMode -eq 1 ];then
    cpudump > cpu_and_thermal.log &
    glogg cpu_and_thermal.log
#fi

#if [ $cpuMode -eq 2 ];then
#    cpuparams > cpu_params.log &
#    glogg cpu_params.log
#fi


