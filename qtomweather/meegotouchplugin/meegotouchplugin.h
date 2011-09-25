#include <QString>
#include <MWidget>
//#include <MContainer>

//#include <duiapplicationextensioninterface.h>
#include <QObject>
#include <MApplicationExtensionInterface>

#include <iostream>
#include <QFile>
#include <fstream>
#include <QTextStream>



class MyMWidget : public MWidget
{
   Q_OBJECT                                                                                                                                                                       
   Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)  
   Q_PROPERTY(QString station READ station NOTIFY stationChanged)  
   Q_PROPERTY(QString temperature READ temperature NOTIFY temperatureChanged)  
public:

    MyMWidget(){
    };

    ~MyMWidget(){};

    Q_INVOKABLE void startapplication(){
        QFile ioFile("/tmp/1.log");

        ioFile.open(QIODevice::Append | QIODevice::Text);
        QTextStream out(&ioFile);
        out<<"sssssssssssssssssssssssssssssss";	
        ioFile.close();
    }

    QString icon(){
        QString c;
	c="/opt/com.meecast.omweather/share/icons/Meecast/28.png";
	return c;
    } 
    QString station(){
        QString c;
	c="Moscow";
	return c;
    } 
    QString temperature(){
        QString c;
	c="London";
	return c;
    }

    void refreshview(){
	    emit iconChanged();
	    emit stationChanged();
	    emit temperatureChanged();
    };

signals:
    void iconChanged();
    void stationChanged();
    void temperatureChanged();
};

class WeatherExtensionInterface : public MApplicationExtensionInterface
{
    Q_INTERFACES(MApplicationExtensionInterface)

public:
    virtual void weatherExtensionSpecificOperation() = 0;
};

Q_DECLARE_INTERFACE(WeatherExtensionInterface, "com.nokia.home.EventsExtensionInterface/1.0")


//class MContainer;
//class MButton;
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
    //MContainer *box;
    //MButton *box;
    //MButton *button;
};

