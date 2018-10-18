<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
  {* FusionDirectory invitation - smarty template *}
  {$php_errors}
  {$msg_dialogs}

  <div id="window-container">

    <div id="window-div">
      <form name="mainform" id="mainform" method="post" enctype="multipart/form-data">
        <div id="window-titlebar">
          <img id="fd-logo" src="geticon.php?context=applications&amp;icon=fusiondirectory&amp;size=48" alt="FusionDirectory"/>
          <p>
            {t}Invitation{/t}
          </p>
        </div>
        <div id="window-content">
          <div>
            <p class="infotext">
              {if $errorMessage}
                {$errorMessage|escape}
              {else}
                {t}Hello{/t}<br/>
                <a href="privateform.php?token={$token|escape}">{t}Click here to connect{/t}</a><br/>
                <a href="publicform.php?token={$token|escape}">{t}Click here if you fail to connect{/t}</a>
              {/if}
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
