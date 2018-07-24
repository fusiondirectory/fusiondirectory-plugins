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
            {$form.fdPublicFormTitle|escape}
          </p>
        </div>
        <div id="window-content">
          <div>
            <p class="infotext">
              {$form.fdPublicFormText|escape}
            </p>

            <br/>
            {$template_dialog}
          </div>
        </div>
        {if !$done}
        <div id="window-footer" class="plugbottom">
          <input type="submit" id="form_submit" name="form_submit" value="{msgPool type=okButton}"/>
        </div>
        {/if}
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
