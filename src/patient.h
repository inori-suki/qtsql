#pragma once

#include <QString>

struct Patient {
    int id = -1;
    QString name;
    QString idCard;
    QString sex;
    QString dob;
    double height = 0.0;
    double weight = 0.0;
    QString mobile;
    int age = 0;
};
