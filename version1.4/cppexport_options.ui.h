
void cppexport_options::on_groupBox_clicked( int )
{

  QString msj;
  if (classnameLineEdit->text().isEmpty())
  {
    msj += tr("Introduzca un nombre para la clase");
  }
  else if (radioButton_app->isChecked())
  {
    msj += QString("soqt-config --build ")+classnameLineEdit->text()+".exe ";
    msj += classnameLineEdit->text()+".cpp\n";
  }
  else if (radioButton_class->isChecked())
  {
    msj += QString("#include \"")+classnameLineEdit->text()+".h\"\n";
    msj += "....\n";
    msj += QString("root->addChild(new ")+classnameLineEdit->text()+"() );\n";
  }
 
  howtoEdit->setText(msj);
  
}
