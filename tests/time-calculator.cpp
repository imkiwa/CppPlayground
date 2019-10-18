#include <iostream>

int main() {
    int year = 2019, month = 9, day = 15; //自定义当前时间

    // 变量名字取得好，就不需要用注释来说明变量的意义
    int deltaYear, deltaMonth, deltaDay;

    for (;;) {
        //提示当前时间
        if (year >= 0) {
            std::cout << "现在是 AD " << year << " 年 " << month << " 月 " << day << " 日"
                      << std::endl;
        } else {
            std::cout << "现在是 BC " << -year << " 年 " << month << " 月 " << day << " 日"
                      << std::endl;
        }

        //输入时间变化
        std::cout << "已经过去了多少年: " << std::endl;
        std::cin >> deltaYear;
        std::cout << "已经过去了多少月: " << std::endl;
        std::cin >> deltaMonth;
        std::cout << "已经过去了多少日: " << std::endl;
        std::cin >> deltaDay;

        year += deltaYear;
        month += deltaMonth;
        day += deltaDay;

        // 输入后先计算冗余月份
        if (month > 12) {
            int temp = (month - 1) / 12;
            year += temp;
            month = month - (12 * temp);
        }

        //计算不合法号数,合法则跳出循环
        for (;;) {
            if (day > 31) {
                if (month == 1) {
                    month++;
                    day -= 31;
                    if (year % 4 != 0 && (year / 400) % 4 != 0) {
                        if (day <= 28)
                            break;
                    } else {
                        if (day <= 29)
                            break;
                    }
                } else if (month == 2) {
                    month++;
                    if (year % 4 != 0 && (year / 400) % 4 != 0) {
                        //非闰年
                        if (day > 28)
                            day -= 28;
                    } else {
                        if (day > 29)
                            day -= 29;
                        if (day <= 30)
                            break;
                    }
                } else if (month == 3) {
                    month++;
                    day -= 31;
                    if (day <= 30)
                        break;
                } else if (month == 4) {
                    month++;
                    day -= 30;
                    if (day <= 31)
                        break;
                } else if (month == 5) {
                    month++;
                    day -= 31;
                    if (day <= 30)
                        break;
                } else if (month == 6) {
                    month++;
                    day -= 30;
                    if (day <= 31)
                        break;
                } else if (month == 7) {
                    month++;
                    day -= 31;
                    if (day <= 31)
                        break;
                } else if (month == 8) {
                    month++;
                    day -= 31;
                    if (day <= 30)
                        break;
                } else if (month == 9) {
                    month++;
                    day -= 30;
                    if (day <= 31)
                        break;
                } else if (month == 10) {
                    month++;
                    day -= 31;
                    if (day <= 30)
                        break;
                } else if (month == 11) {
                    month++;
                    day -= 30;
                    if (day <= 31)
                        break;
                } else if (month == 12) {
                    month++;
                    day -= 31;
                    if (day <= 31)
                        break;
                } else {
                    //解决月份负数问题
                    int temp = month / 12;
                    --temp;
                    year += temp;
                    month += (-temp * 12);
                }

                if (month > 12) {
                    ++year;
                    month -= 12;
                }

            } else if (day <= 0) {
                --month;
                if (month == 1) {
                    if (year % 4 != 0 && (year / 400) % 4 != 0) {
                        day += 28;
                        break;
                    } else {
                        day += 29;
                        break;
                    }
                } else if (month == 2) {
                    day += 31;
                    break;
                } else if (month == 3) {
                    day += 30;
                    break;
                } else if (month == 4) {
                    day += 31;
                    break;
                } else if (month == 5) {
                    day += 30;
                    break;
                } else if (month == 6) {
                    day += 31;
                    break;
                } else if (month == 7) {
                    day += 31;
                    break;
                } else if (month == 8) {
                    day += 30;
                    break;
                } else if (month == 9) {
                    day += 31;
                    break;
                } else if (month == 10) {
                    day += 30;
                    break;
                } else if (month == 11) {
                    day += 31;
                    break;
                } else if (month == 12) {
                    day += 31;
                    break;
                }

            } else {
                if (day == 31) {
                    if (month == 4 || month == 6 || month == 9 || month == 11) {
                        ++month;
                        day -= 30;
                        break;
                    } else if ((year % 4 != 0 && (year / 400) % 4 != 0) && month == 2) {
                        ++month;
                        day -= 28;
                        break;
                    } else if ((year % 400 == 0
                                || (year % 4 == 0 && (year / 400) % 4 != 0))
                               && month == 2) {
                        //闰年
                        ++month;
                        day -= 29;
                        break;
                    }
                } else if (day == 29) {
                    if ((year % 4 != 0 && (year / 400) % 4 != 0) && month == 2) {
                        ++month;
                        day -= 28;
                        break;
                    }
                } else if (day == 30) {
                    if ((year % 400 == 0 || (year % 4 == 0 && (year / 400) % 4 != 0)) &&
                        month == 2) {
                        //闰年
                        ++month;
                        day -= 29;
                        break;
                    } else if ((year % 4 != 0 && (year / 400) % 4 != 0) && month == 2) {
                        ++month;
                        day -= 28;
                        break;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
        }

        if (year >= 0) {
            std::cout << "现在是 AD " << year << " 年 " << month << " 月 " << day << " 日"
                      << std::endl;
        } else {
            std::cout << "现在是 BC " << -year << " 年 " << month << " 月 " << day << " 日"
                      << std::endl;
        }

        std::cout << "是否退出? (y/n) " << std::endl;
        char a;
        std::cin >> a;
        if (a == 'y' || a == 'Y') {
            break;
        }
    }
    return 0;
}

