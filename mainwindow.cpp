#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //設定ファイル読み込み
    QSettings settings("order_directory.ini", QSettings::IniFormat);
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
    QSettings settings("order_directory.ini", QSettings::IniFormat);
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

    //数値チェック
    for (int i =0;i<orderNum.size();i++)
    {
       if (!orderNum[i].isDigit()){
            QMessageBox::critical(this,tr("Error"),tr("伝票番号は全て数字である必要があります。"));
            return;
        }
    }
    if(orderNum.size() < 10){
        QMessageBox::critical(this,tr("Error"),tr("伝票番号は１０桁以上です。"));
        return;
    }

    QString orderDir = orderNum.left(4) + "/" + orderNum.mid(4, 2) + "/" + orderNum.right(orderNum.length() - 6);

    QString path = root + '/' + orderDir;

    //存在しない場合は作成
    QDir dir(path);
    if (!dir.exists()){
        dir.mkpath(".");
    }

    //ディレクトリ開く
    QDesktopServices::openUrl( QUrl::fromLocalFile ( path ));

    //txtEditのクリア
    ui->orderNumberEdit->clear();
}
