/* vim: set sw=4 ts=4 et: */
/*
 * This file is part of Other Maemo Weather(omweather)
 *
 * Copyright (C) 2006-2011 Vlad Vasiliev
 * Copyright (C) 2010-2011 Tanya Makova
 *     for the code
 *
 * Copyright (C) 2008 Andrew Zhilin
 *		      az@pocketpcrussia.com 
 *	for default icon set (Glance)
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU weather-config.h General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
*/
/*******************************************************************************/
#include <QString>
#include <MWidget>
#include <QLabel>
#include <MImageWidget>
#include <QObject>
#include <MApplicationExtensionInterface>
#include <MGConfItem>
#include <QProcess>
#include <QDir>
#include <QGraphicsAnchorLayout>
// Debug
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTimer>



class MyMWidget : public MWidget
{
   Q_OBJECT                                                                                                                                                                       
private:
    QProcess process;
    QString  _stationname;
    QString  _temperature;
    QString  _temperature_high;
    QString  _temperature_low;
    QString  _iconpath;
    QString  _lastupdate;
    bool    _current;
    bool    _lockscreen;
    bool    _standbyscreen;
    QTimer  *_timer;
    MGConfItem *_wallpaperItem;
    MGConfItem *_standbyItem;
    MGConfItem *_original_wallpaperItem;
    QString _wallpaper_path;
    QImage *_image;
    MImageWidget *_icon;
    QImage *_events_image;
    bool _down;
public:

    MyMWidget(){

        
#if 0
    QFile file("/tmp/1.log");
    if (file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)){
	    QTextStream out(&file);
	    out <<  "Begin PreInit MyWidget ."<<".\n";
	    file.close();
	}
#endif

      _stationname = "Unknown";
      _temperature = "";
      _temperature_low = "";
      _temperature_high = "";
      _iconpath = "/opt/com.meecast.omweather/share/icons/Meecast/49.png";
      _current = false;
      _lockscreen = false;
      _standbyscreen = false;
      _timer = new QTimer(this);
      _timer->setSingleShot(true);
      _down = false;
      
      /* preparing for events widget */ 
      QGraphicsAnchorLayout *layout = new QGraphicsAnchorLayout();
      _events_image = new QImage (QSize(127, 96), QImage::Format_ARGB32);
      _events_image->load("/opt/com.meecast.omweather/share/icons/Meecast/49.png");
      *_events_image = _events_image->scaled(127, 96);
      _icon = new MImageWidget(_events_image);
      grabMouse();

      layout->addAnchor(layout, Qt::AnchorHorizontalCenter, _icon, Qt::AnchorHorizontalCenter);
      layout->setContentsMargins(1, 1, 1, 1);
      layout->setSpacing(0);
      setLayout(layout);
    
      /* preparing for standby screen */
      _standbyItem = new MGConfItem ("/desktop/meego/screen_lock/low_power_mode/operator_logo"); 
      connect(_standbyItem, SIGNAL(valueChanged()), this, SLOT(updateStandbyPath()));
      /* preparing for wallpaper widget */
      _wallpaperItem = new MGConfItem ("/desktop/meego/background/portrait/picture_filename"); 
      connect(_wallpaperItem, SIGNAL(valueChanged()), this, SLOT(updateWallpaperPath()));
      if (!_wallpaperItem || _wallpaperItem->value() == QVariant::Invalid)
        _wallpaper_path = "/home/user/.wallpapers/wallpaper.png";
      else{
#if 0
          // Debug begin
	if (file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)){
	    QTextStream out(&file);
	    out <<  "PreInit MyWidget ."<<_wallpaperItem->value().toString()<<".\n";
	    file.close();
	}
#endif
        _wallpaper_path = _wallpaperItem->value().toString();
        if (_wallpaper_path.indexOf("MeeCast",0) != -1){
            _wallpaper_path = "/home/user/.cache/com.meecast.omweather/wallpaper_MeeCast_original.png";
        }
      }
      _image = new QImage;
      _image->load(_wallpaper_path);
      if (_image->dotsPerMeterX() != 3780 || _image->dotsPerMeterY() != 3780 ){
        _image->setDotsPerMeterX(3780);
        _image->setDotsPerMeterY(3780);
      }
      if (_wallpaper_path.indexOf("MeeCast",0) == -1){
        _image->save("/home/user/.cache/com.meecast.omweather/wallpaper_MeeCast_original.png");
      }

      connect(_timer, SIGNAL(timeout()), this, SLOT(update_data()));
#if 0
    // Debug begin
	if (file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)){
	    QTextStream out(&file);
	    out <<  "Finish Init MyWidget ."<<_wallpaper_path<<".\n";
	    file.close();
	}
#endif
    };

     ~MyMWidget(){
      delete _timer;
    };
   
    void
    mousePressEvent(QGraphicsSceneMouseEvent *event){
        _down = true;
    }

    void
    mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
        if (_down)
           startapplication();
        _down = false;
    }

    void refreshwallpaper(bool new_wallpaper = false);
    void refresheventswidget(void);
    void refreshstandby(void);

    Q_INVOKABLE void startapplication(){
        QString executable("/usr/bin/invoker");    
        QStringList arguments;
        arguments << "--single-instance";
        arguments << "--splash=/home/user/.cache/com.meecast.omweather/splash.png";
        arguments << "--type=d";
        arguments <<"/opt/com.meecast.omweather/bin/omweather-qml";	
        process.startDetached(executable, arguments);
    }

    Q_INVOKABLE void startpredeamon(){
#if 0
	// Debug begin
	QFile file("/tmp/1.log");
	if (file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)){
	    QTextStream out(&file);
	    out <<  QLocale::system().toString(QDateTime::currentDateTime(), QLocale::LongFormat) << "startpredeamon"<< _stationname<< "\n";
	    file.close();
	}
	// Debug end 
#endif
        QString executable("/opt/com.meecast.omweather/bin/predaemon");    
        process.startDetached(executable);
    }


    void icon(const QString &iconpath){
	    _iconpath = iconpath;
    }

    QString icon(){
	    return _iconpath; 
    } 

    void station(const QString &stationname){
	    _stationname = stationname;
    }

    QString station(){
	    return _stationname;
    } 

    void temperature(const QString &temperature){
	    _temperature = temperature;
    }

    QString temperature(){
	    return _temperature;
    }

    void temperature_high(const QString &temperature){
	    _temperature_high = temperature;
    }

    QString temperature_high(){
	    return _temperature_high;
    }

    void temperature_low(const QString &temperature){
	    _temperature_low = temperature;
    }

    QString temperature_low(){
	    return _temperature_low;
    }

    void lastupdate(const QString &lastupdate){
	    _lastupdate = lastupdate;
    }

    QString lastupdate(){
	    return _lastupdate;
    } 

    void current(bool cur){
        _current = cur;
    }

    bool current(){
        return _current;
    }

    void lockscreen(bool cur){
        _lockscreen = cur;
    }
    bool lockscreen(){
        return _lockscreen;
    }
    void standbyscreen(bool cur){
        _standbyscreen = cur;
    }
    bool syandbyscreen(){
        return _standbyscreen;
    }

    void refreshview(){
#if 0
        // Debug begin
        QFile file("/tmp/1.log");
        if (file.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream out(&file);
            out <<  QLocale::system().toString(QDateTime::currentDateTime(), QLocale::LongFormat) << "refreshview"<< " \n";
            file.close();
        }
        // Debug end 
#endif
          refresheventswidget();
          refreshwallpaper();           
          if (_standbyscreen)
              refreshstandby();
	   
    };

public Q_SLOTS:
    void SetCurrentData(const QString &station, const QString &temperature, const QString &temperature_high, const QString &temperature_low, const QString &icon, 
                        const uint until_valid_time, bool current, bool lockscreen_param, bool standbyscreen_param, const QString &last_update);
    void update_data();
    void refreshRequested();
    void updateWallpaperPath();
    void updateStandbyPath();
signals:
    void iconChanged();
    void stationChanged();
    void temperatureChanged();
    void temperature_highChanged();
    void temperature_lowChanged();
    void currentChanged();
};

class WeatherExtensionInterface : public MApplicationExtensionInterface
{
    Q_INTERFACES(MApplicationExtensionInterface)

public:
    virtual void weatherExtensionSpecificOperation() = 0;
};

Q_DECLARE_INTERFACE(WeatherExtensionInterface, "com.nokia.home.EventsExtensionInterface/1.0")


class MyMWidget;

class WeatherApplicationExtension : public QObject, public WeatherExtensionInterface
{
    Q_OBJECT
    Q_INTERFACES(WeatherExtensionInterface MApplicationExtensionInterface)

public:
    WeatherApplicationExtension();
    virtual ~WeatherApplicationExtension();

    virtual void weatherExtensionSpecificOperation();

    virtual bool initialize(const QString &interface);
    virtual MWidget *widget();

private:
    MyMWidget *box;
};

