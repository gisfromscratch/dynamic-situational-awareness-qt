#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "MessageSimulatorController.h"

void printHelp()
{
  QTextStream out(stdout);
  out << "Available command line parameters:" << endl;
  out << "  -h                     Print help and exit" << endl;
  out << "  -c                     Console mode (no GUI)" << endl;
  out << "Parameters available only in console mode:" << endl;
  out << "  -p <port number>       Port number: Required" << endl;
  out << "  -f <filename>          Simulation file: Required" << endl;
  out << "  -q <frequency>         Frequency (messages per time unit); default is 1.0" << endl;
  out << "  -t <time unit>         Time unit for frequency; valid values are seconds," << endl <<
         "                         minute, and hour; default is second" << endl;
  out << "  -l                     Simulation loops through simulation file" << endl;
  out << "  -s                     Silent mode; no verbose output" << endl;
}

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("Esri");
  QCoreApplication::setOrganizationDomain("esri.com");
  QCoreApplication::setApplicationName("MessageSimulator");

  bool isGui = true;
  QString simulationFile;
  int port = -1;
  float frequency = 1.0f;
  QString timeUnit = "second";
  bool isLoop = false;
  bool isVerbose = true;

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-h"))
    {
      // Print help and exit
      printHelp();
      return 0;
    }
    else if (!strcmp(argv[i], "-c"))
    {
      isGui = false;
    }
    else if (!strcmp(argv[i], "-f"))
    {
      if ((i + 1) < argc)
      {
        simulationFile = QString(argv[++i]);
      }
    }
    else if (!strcmp(argv[i], "-p"))
    {
      if ((i + 1) < argc)
      {
        port = atoi(argv[++i]);
      }
    }
    else if (!strcmp(argv[i], "-q"))
    {
      if ((i + 1) < argc)
      {
        frequency = atof(argv[++i]);
      }
    }
    else if (!strcmp(argv[i], "-t"))
    {
      if ((i + 1) < argc)
      {
        timeUnit = QString(argv[++i]);
      }
    }
    else if (!strcmp(argv[i], "-l"))
    {
      isLoop = true;
    }
    else if (!strcmp(argv[i], "-s"))
    {
      isVerbose = false;
    }
  }

  if (!isGui)
  {
    if (simulationFile.isEmpty() || port == -1)
    {
      printHelp();
      return 0;
    }

    QCoreApplication app(argc, argv);

    MessageSimulatorController controller;

    if (isVerbose)
    {
      QObject::connect(&controller, &MessageSimulatorController::errorOccurred, &app, [](const QString& error)
      {
        qDebug() << error;
      });
    }

    controller.setMessageFrequency(frequency);
    controller.setTimeUnit(MessageSimulatorController::toTimeUnit(timeUnit));
    controller.setPort(port);
    controller.setSimulationLooped(isLoop);
    controller.startSimulation(QUrl::fromLocalFile(simulationFile));

    if (isVerbose)
    {
      qDebug() << "Simulation started with file: " << controller.simulationFile().toString();
      qDebug() << "UDP port: " << controller.port();
      qDebug() << "Sending " << controller.messageFrequency() << " message per " <<
                  MessageSimulatorController::fromTimeUnit(controller.timeUnit());
      if (isLoop)
        qDebug() << "Simulation loop mode enabled";
    }

    return app.exec();
  }
  else
  {
    QGuiApplication app(argc, argv);

    qmlRegisterType<MessageSimulatorController>("Esri.MessageSimulator", 1, 0, "MessageSimulatorController");
    qRegisterMetaType<MessageSimulatorController::TimeUnit>("MessageSimulatorController::TimeUnit");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
  }
}
