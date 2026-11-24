#pragma once

#include <QString>

inline QString appStyleSheet()
{
    return QStringLiteral(R"(
* {
    font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
    font-size: 13px;
    color: #23384D;
}
QMainWindow, QWidget#LoginPage, QWidget#WorkspacePage {
    background: #F3F7FB;
}
QFrame#BrandPanel {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #12345A, stop:1 #1F6F8B);
    border: none;
}
QLabel#BrandKicker {
    color: #9FD3E3;
    font-size: 11px;
    font-weight: 600;
}
QLabel#BrandTitle {
    color: #FFFFFF;
    font-size: 34px;
    font-weight: 700;
}
QLabel#BrandSubtitle {
    color: #D3E9F1;
    font-size: 14px;
}
QLabel#BrandBullet {
    color: #EAF6FA;
    font-size: 13px;
}
QLabel#BrandFooter {
    color: #9FC7D6;
    font-size: 11px;
}
QFrame#LoginCard {
    background: #FFFFFF;
    border: 1px solid #E1E9F1;
    border-radius: 18px;
}
QLabel#LoginTitle {
    color: #12344D;
    font-size: 26px;
    font-weight: 700;
}
QLabel#LoginSubtitle {
    color: #7A8A99;
    font-size: 12px;
}
QLabel#FieldLabel {
    color: #4C5F70;
    font-size: 12px;
    font-weight: 600;
}
QLineEdit, QComboBox, QDateEdit, QSpinBox, QDoubleSpinBox {
    background: #FBFCFE;
    border: 1px solid #D7E2EC;
    border-radius: 8px;
    padding: 8px 10px;
    min-height: 20px;
    selection-background-color: #BFE3EA;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus,
QSpinBox:focus, QDoubleSpinBox:focus {
    border: 1px solid #2A9D8F;
    background: #FFFFFF;
}
QLineEdit#Input {
    min-height: 22px;
    font-size: 14px;
}
QPushButton {
    border-radius: 8px;
    padding: 8px 16px;
    border: 1px solid #D7E2EC;
    background: #FFFFFF;
    color: #334E68;
}
QPushButton:hover {
    background: #F1F6FA;
}
QPushButton#PrimaryButton {
    background: #1F6F8B;
    border: 1px solid #1F6F8B;
    color: #FFFFFF;
    font-weight: 600;
    min-height: 24px;
}
QPushButton#PrimaryButton:hover {
    background: #185A72;
}
QPushButton#SecondaryButton {
    background: #E8F3F6;
    border: 1px solid #CFE4EA;
    color: #1F6F8B;
    font-weight: 600;
}
QPushButton#SecondaryButton:hover {
    background: #DCEEF3;
}
QPushButton#DangerButton {
    background: #D64545;
    border: 1px solid #D64545;
    color: #FFFFFF;
    font-weight: 600;
}
QPushButton#DangerButton:hover {
    background: #B83232;
}
QPushButton#GhostButton {
    background: transparent;
    border: 1px solid #D7E2EC;
    color: #52616F;
}
QPushButton#GhostButton:hover {
    background: #F1F6FA;
}
QLabel#ErrorLabel {
    color: #C0392B;
    font-size: 12px;
}
QLabel#ErrorBanner {
    background: #FDECEC;
    border: 1px solid #F3C4C1;
    border-radius: 7px;
    color: #B42318;
    padding: 8px 10px;
    font-size: 12px;
}
QLabel#HintLabel {
    color: #8A99A8;
    font-size: 11px;
}
QLabel#DeveloperLabel {
    color: #8A99A8;
    font-size: 11px;
}
QFrame#HeaderBar {
    background: #FFFFFF;
    border: none;
    border-bottom: 1px solid #E1E9F1;
}
QLabel#AppTitle {
    color: #12344D;
    font-size: 19px;
    font-weight: 700;
}
QLabel#AppSubtitle {
    color: #8A99A8;
    font-size: 11px;
}
QLabel#UserBadge {
    background: #EAF5F2;
    color: #1F6F5E;
    border-radius: 10px;
    padding: 5px 12px;
    font-size: 12px;
    font-weight: 600;
}
QFrame#Sidebar {
    background: #FFFFFF;
    border: none;
    border-right: 1px solid #E1E9F1;
}
QLabel#SidebarSection {
    color: #9AA9B7;
    font-size: 10px;
    font-weight: 600;
}
QPushButton#NavButton {
    text-align: left;
    border: none;
    border-radius: 8px;
    padding: 11px 14px;
    color: #5C6B7A;
    background: transparent;
    font-size: 13px;
}
QPushButton#NavButton:hover {
    background: #F1F6FA;
    color: #1F6F8B;
}
QPushButton#NavButton:checked {
    background: #E7F2F6;
    color: #1F6F8B;
    font-weight: 700;
}
QFrame#Card {
    background: #FFFFFF;
    border: 1px solid #E1E9F1;
    border-radius: 12px;
}
QLabel#PageTitle {
    color: #12344D;
    font-size: 21px;
    font-weight: 700;
}
QLabel#PageSubtitle {
    color: #7A8A99;
    font-size: 12px;
}
QLabel#CountLabel {
    color: #1F6F8B;
    background: #E7F2F6;
    border-radius: 10px;
    padding: 5px 11px;
    font-size: 11px;
    font-weight: 600;
}
QLabel#StatusLabel {
    color: #7A8A99;
    font-size: 11px;
}
QTableView {
    background: #FFFFFF;
    alternate-background-color: #F8FAFC;
    border: none;
    gridline-color: #EEF2F6;
    selection-background-color: #E3F0F4;
    selection-color: #12344D;
}
QTableView::item {
    padding: 6px 8px;
    border-bottom: 1px solid #EEF2F6;
}
QHeaderView::section {
    background: #F5F8FB;
    color: #52616F;
    border: none;
    border-bottom: 1px solid #E1E9F1;
    padding: 9px 8px;
    font-size: 12px;
    font-weight: 600;
}
QScrollBar:vertical {
    background: transparent;
    width: 9px;
    margin: 2px;
}
QScrollBar::handle:vertical {
    background: #C7D5E1;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}
QDialog {
    background: #F7FAFC;
}
QLabel#DialogTitle {
    color: #12344D;
    font-size: 19px;
    font-weight: 700;
}
QLabel#DialogSubtitle {
    color: #7A8A99;
    font-size: 11px;
}
QLabel#WarnIcon {
    background: #FDF1E7;
    color: #D97706;
    border-radius: 26px;
    font-size: 28px;
    font-weight: 700;
}
QLabel#DangerIcon {
    background: #FDECEC;
    color: #D64545;
    border-radius: 26px;
    font-size: 26px;
    font-weight: 700;
}
)");
}
