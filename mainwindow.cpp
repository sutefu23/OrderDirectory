#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtDebug>
#include <time.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //設定ファイル読み込み
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QSettings settings(path+"/order_directory.ini", QSettings::IniFormat);

    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    settings.beginGroup("CONFIG");
    int windowX = settings.value("POSITION_X",50).toInt();
    int windowY = settings.value("POSITION_Y",50).toInt();
    QString rootPath = settings.value("ROOT_PATH","").toString();

    this->setGeometry(windowX, windowY, 570, 320);

    //ルートパスデフォルト値
    ui->rootPathEdit->setText(rootPath);

    // 伝票テキストボックスにフォーカス
    ui->orderNumberEdit->setFocus();

}

//ウィンドウインスタンス破棄時
MainWindow::~MainWindow()
{
    //ウィンドウ位置保存
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QSettings settings(path+"/order_directory.ini", QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    settings.beginGroup("CONFIG");
    settings.setValue("POSITION_X",this->x());
    settings.setValue("POSITION_Y",this->y());
    settings.setValue("ROOT_PATH",ui->rootPathEdit->text());
    delete ui;
}

//ディレクトリ設定ボタン押下時
void MainWindow::on_openDirectoryButton_clicked()
{
    //ルートディレクトリのパス設定
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::Directory);
    fileDialog.setOption(QFileDialog::ShowDirsOnly, true);
    if(fileDialog.exec()){
        QStringList filePaths = fileDialog.selectedFiles();
        ui->rootPathEdit->setText(filePaths[0]);
    }
}
// 伝票番号LineEdit return押下時
void MainWindow::on_orderNumberEdit_returnPressed()
{

    //伝票番号のディレクトリを開く
    QString root = ui->rootPathEdit->text();
    QString orderNum = ui->orderNumberEdit->text();

    if(root.isEmpty()){
        QMessageBox::critical(this,tr("Error"),tr("ルートディレクトリが設定されていません。"));
        return;
    }
    if(orderNum.isEmpty()){
        QMessageBox::critical(this,tr("Error"),tr("伝票番号が記載されていません。"));
        return;
    }

    //数値ではない場合、今月で新規作成して離脱
    bool isDegit;
    orderNum.toInt(&isDegit);
    if(!isDegit){
        time_t theTime = time(NULL);
        struct tm *current = localtime(&theTime);
        int year = current->tm_year + 1900;
        int month = current->tm_mon + 1;

        QString orderDir = QString::number(year) + "/" + QString::number(month) + "/" + orderNum;
        QString createPath = root + '/' + orderDir;
        QDir createDir(createPath);
        if (!createDir.exists()){
            if(!ui->createDirectoryCheck->isChecked()){//新規作成しないモードon
                QMessageBox::warning(this,tr("Notice"),tr("伝票フォルダが見つかりません"));
                return;
            }
            createDir.mkpath(".");
        }

        //ディレクトリ開く
        QDesktopServices::openUrl( QUrl::fromLocalFile ( createPath ));
        //txtEditのクリア
        ui->orderNumberEdit->clear();
        return;
    }


    if(orderNum.size() != 6 && orderNum.size() != 8 && orderNum.size() != 9){
        QMessageBox::critical(this,tr("Error"),tr("伝票番号は６桁,８桁,９桁です。"));
        return;
    }


    bool isLegacyOrderType = orderNum.size() == 6;//最後が６桁の時は昔の伝票番号形式


    //txtEditのクリア
    ui->orderNumberEdit->clear();


    if(isLegacyOrderType){
        int fromYear = 2012;
        QDir rootDir(root);

        foreach(QString yearDirName , rootDir.entryList(QDir::Filter::AllDirs, QDir::SortFlag::Name)){
            bool isDigit;
            if (yearDirName.toInt(&isDigit) > fromYear){
                continue;
            }
            if(!isDigit) continue;

            QDir yearDir(root + '/' + yearDirName);
            foreach(QString monthDirName , yearDir.entryList(QDir::Filter::AllDirs, QDir::SortFlag::Name)){
                monthDirName.toInt(&isDigit);
                if(!isDigit) continue;

                QString orderDir = yearDirName + '/' + monthDirName + '/' + orderNum;
                QString searchPath = root + '/' + orderDir;
                QDir searchDir(searchPath);
                qDebug() << searchPath;
                if(searchDir.exists()){
                    //ディレクトリ開く
                    QDesktopServices::openUrl( QUrl::fromLocalFile ( searchPath ));
                    return;
                }
            }
        }
        //捜査しても見つからない
        QMessageBox::warning(this,tr("Notice"),tr("伝票フォルダが見つかりません"));
        return;

    }else{//通常伝票
        QString orderDir = "20" + orderNum.left(2) + "/" + orderNum.mid(2, 2) + "/" + orderNum.right(orderNum.length() - 4);
        QString searchPath = root + '/' + orderDir;
        //存在しない場合は作成
        QDir searchDir(searchPath);
        if (!searchDir.exists()){
            if(!ui->createDirectoryCheck->isChecked()){//新規作成しないモードon
                QMessageBox::warning(this,tr("Notice"),tr("伝票フォルダが見つかりません"));
                return;
            }
            searchDir.mkpath(".");
        }
        //ディレクトリ開く
        QDesktopServices::openUrl( QUrl::fromLocalFile ( searchPath ));
        return;
    }



}
