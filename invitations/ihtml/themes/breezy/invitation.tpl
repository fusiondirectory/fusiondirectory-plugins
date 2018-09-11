<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
  {* FusionDirectory public form - smarty template *}
  {$php_errors}
  {$msg_dialogs}

  <div id="window-container">

    <div id="window-div">
      <form action="" name="mainform" id="mainform" method="post" enctype="multipart/form-data">
        <div id="window-titlebar">
          <img id="fd-logo" src="geticon.php?context=applications&amp;icon=fusiondirectory&amp;size=48" alt="FusionDirectory"/>
          <p>
            Invitation
          </p>
        </div>
        <div id="window-content">
          <div>
            <p class="infotext">
              Hello.<br/>
              <a href="privateform.php?token={$token|escape}">Click here to connect</a><br/>
              <a href="publicform.php?token={$token|escape}">Click here if you fail to connect</a>
            </p>
          </div>
        </div>
        <input type="hidden" name="CSRFtoken" value="{$CSRFtoken}"/>
      </form>
    </div>
  </div>
  <script type="text/javascript">
    <!-- // Error Popup
    next_msg_dialog();
    -->
  </script>
</body>
</html>
