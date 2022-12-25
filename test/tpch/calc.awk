#! /usr/bin/awk -f
{
    sum += $1                   # 假设数据放在第一列
    nums[NR] = $1  # 将数据记录到数组中
}
END {
    if (NR == 0) exit  #防止出现处于0的情况To avoid division by zero

    asort(nums)  #  先对数据进行排序，用于记录中位数

    # 计算中位数
    median = (NR % 2 == 0) ? ( nums[NR / 2] + nums[NR / 2 + 1] ) / 2  : nums[int(NR / 2) + 1]

    # 计算平均
    mean = sum/NR

    printf "min = %s, max = %s, median = %s, mean = %s\n", nums[1], nums[NR], median, mean
}