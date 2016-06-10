<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
  {* FusionDirectory user reminder - smarty template *}
  {$php_errors}
  <div>
    {include file={filePath file="setup_header.tpl"}}
    <div class='setup_menu'>
      <b>{t}FusionDirectory expiration postpone{/t}</b>
    </div>
  </div>

  <div id="window_container">

    <div id="window_div">
        {$msg_dialogs}
        <div id="window_titlebar">
          <p>
            <img class="center" src="geticon.php?context=types&amp;icon=user&amp;size=48" alt="{t}User{/t}" title="{t}User{/t}"/>
            {t}Expiration postpone{/t}
          </p>
        </div>
        <div style="padding-left:10px;padding-right:10px;">
          <!-- Display SSL warning message on demand -->
          <p class="warning">{$ssl}</p>

          <!-- Display error message on demand -->
          <p class="warning">{$message}</p>
          {if $success}
            <div class="success">
              <img class="center" src="geticon.php?context=status&amp;icon=task-complete&amp;size=16" alt="{t}Success{/t}" title="{t}Success{/t}">&nbsp;
              <b>{t}Your expiration has been postponed successfully.{/t}</b><br/>
              <br/><a href="./">Return to login screen</a>
            </div>
          {else}
            <div class="error">
              <img class="center" src="geticon.php?context=status&amp;icon=task-failure&amp;size=16" alt="{t}Error{/t}" title="{t}Error{/t}">&nbsp;
              <b>{t}There was a problem{/t}</b>
            </div>
          {/if}
        </div>
    </div>
  </div>
</body>
  <script type="text/javascript">
    <!-- // Error Popup
    next_msg_dialog();
    -->
  </script>
</html>
