#include "configqml.h"

ConfigQml::ConfigQml():QObject(),Core::Config(){

}

QString
ConfigQml::iconset(){
    QString c;
    c = ConfigQml::Config::Iconset().c_str();
    return c;
}

QString
ConfigQml::iconspath(){
    QString c;
    c = ConfigQml::Config::Base_Icons_Path().c_str();
    return c;
}

QString
ConfigQml::temperatureunit(){
    QString c;
    c = ConfigQml::Config::TemperatureUnit().c_str();
    return c;
}
QColor
ConfigQml::fontcolor(){
    QColor c;
    c.setNamedColor(ConfigQml::Config::FontColor().c_str());
    return c;
}
void
ConfigQml::refreshconfig(){
    emit ConfigQml::iconsetChanged();
    emit ConfigQml::iconspathChanged();
    emit ConfigQml::temperatureunitChanged();
    emit ConfigQml::fontcolorChanged();
}

