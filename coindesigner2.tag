<?xml version='1.0' encoding='ISO-8859-1' standalone='yes' ?>
<tagfile>
  <compound kind="class">
    <name>cds_editor</name>
    <filename>classcds__editor.html</filename>
    <member kind="slot">
      <type>void</type>
      <name>on_actionChanged</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>7a394f7e7a96a2469a84e5e37a71be0a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="slot">
      <type>void</type>
      <name>cambiarPicado</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>a179dc55074d4fea3f859d100f24d530</anchor>
      <arglist>(QAction *id)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>cds_editor</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>9edc0cd7fe5dae987041331c34aabbbe</anchor>
      <arglist>(QWidget *p=0, Qt::WindowFlags f=0)</arglist>
    </member>
    <member kind="variable">
      <type>Ui::cds_editor</type>
      <name>Ui</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>2999f5ac26d76eefa6d3ccc010033ff4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>QAction *</type>
      <name>pickAction</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>1a29d80b36505ec096f5e1729a924225</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SoSeparator *</type>
      <name>myRoot</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>66ce987e258beb3c75b38427e4eb87f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>SoSeparator *</type>
      <name>mark_sep</name>
      <anchorfile>classcds__editor.html</anchorfile>
      <anchor>f050b0a81eb1684ae7bb888b0b0fb4d3</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>CdsEditorTemplate</name>
    <filename>classCdsEditorTemplate.html</filename>
    <templarg>SOTYPEVIEWER</templarg>
    <base>cds_editor</base>
    <member kind="function">
      <type></type>
      <name>CdsEditorTemplate</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>c04a940106da425e8ddb1c59b425d473</anchor>
      <arglist>(SoNode *root=NULL)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>setSceneGraph</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>e3df72e7a3c244efdb0ad4cd1abe2c23</anchor>
      <arglist>(SoSeparator *node)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>showMarks</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>be100f5d6d4b489654cb9ff7b7a191da</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>hideMarks</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>d5d0299d9a1f964d38a68c5c3b4ba365</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>show</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>bb8bc1cdf5e38bd3f05b533fae8a364a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>SbBool</type>
      <name>processSoEvent</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>8bfe8ff8afbf130708be1238c4581a65</anchor>
      <arglist>(const SoEvent *const event)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>pickCallback</name>
      <anchorfile>classCdsEditorTemplate.html</anchorfile>
      <anchor>7b6f2a5ed040db48d8a09cf35a808b0d</anchor>
      <arglist>(SoEventCallback *n)</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>CdsNoQuitTemplate</name>
    <filename>classCdsNoQuitTemplate.html</filename>
    <templarg>SOTYPEVIEWER</templarg>
    <member kind="function">
      <type></type>
      <name>CdsNoQuitTemplate</name>
      <anchorfile>classCdsNoQuitTemplate.html</anchorfile>
      <anchor>a50c3bcb6ac0c8c9d357a44a9976cd92</anchor>
      <arglist>(QWidget *parent=NULL, QWidget *_closeWidget=NULL, SoNode *root=NULL)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>~CdsNoQuitTemplate</name>
      <anchorfile>classCdsNoQuitTemplate.html</anchorfile>
      <anchor>4cab9d86bdabce1a799b194bf6297a26</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>SbBool</type>
      <name>processSoEvent</name>
      <anchorfile>classCdsNoQuitTemplate.html</anchorfile>
      <anchor>9182269c147198bf57e4234db7c4fffa</anchor>
      <arglist>(const SoEvent *const event)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>closeWindow</name>
      <anchorfile>classCdsNoQuitTemplate.html</anchorfile>
      <anchor>8074218fd9d0830861f2b3c752e296b0</anchor>
      <arglist>()</arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>QDumper</name>
    <filename>classQDumper.html</filename>
  </compound>
</tagfile>
