<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">
  {* FusionDirectory public form - smarty template *}
  {$php_errors}
  {$msg_dialogs}

  <div id="window-container">

    <div id="window-div">
      <form name="mainform" id="mainform" method="post" enctype="multipart/form-data">
        <div id="window-titlebar">
          <img id="fd-logo" src="geticon.php?context=applications&amp;icon=fusiondirectory&amp;size=48" alt="FusionDirectory"/>
          <p>
            {$form.fdPublicFormTitle|escape}
          </p>
        </div>
        <div id="window-content">
          <div>
            {if $done}
              <p class="infotext">
                {if $errorMessage}
                  {$errorMessage|escape}
                {else}
                  {$form.fdPublicFormFinalText|escape}
                {/if}
              </p>
            {else}
              <p class="infotext">
                {$form.fdPublicFormText|escape}
              </p>

              <br/>
              {$template_dialog}
            {/if}
          </div>
          {if (!$done and $form.fdPublicFormTosUrl)}
            <label for="tosCheckBox"><input type="checkbox" id="tosCheckBox" name="tosCheckBox"/> {t 1=$form.fdPublicFormTosUrl escape=no}I agree to the <a target="_blank" href="%1">terms of service</a>{/t}</label>
          {/if}
          {if (!$done and $captcha)}
            <label for="captcha_code">
              <div id="captcha_container" style="width:255px;margin:auto;">
                <img id="captcha_image" style="float:left;" src="?captcha=show&amp;75aace5fbae3ff8717add401e4291cbb" alt="CAPTCHA Image" />
                <div id="captcha_image_audio_div">
                  <audio id="captcha_image_audio" preload="none" style="display: none">
                    <source id="captcha_image_source_wav" src="?captcha=play&amp;id=5da816517fa90" type="audio/wav">
                  </audio>
                </div>
                <div id="captcha_image_audio_controls">
                  <a class="captcha_play_button" href="?captcha=play&amp;id=5da816517fad3" onclick="return false">
                    <img class="captcha_play_image" height="32" width="32" src="{$captcha.securimage_uri}/images/audio_icon.png" alt="Play CAPTCHA Audio"/>
                    <img class="captcha_loading_image rotating" height="32" width="32" src="{$captcha.securimage_uri}/images/loading.png" alt="Loading audio" style="display: none"/>
                  </a>
                  <noscript>Enable Javascript for audio controls</noscript>
                </div>
                <script type="text/javascript" src="{$captcha.securimage_uri}/securimage.js"></script>
                <script type="text/javascript">captcha_image_audioObj = new SecurimageAudio({ audioElement: 'captcha_image_audio', controlsElement: 'captcha_image_audio_controls' });</script>
                <a href="#" title="Refresh Image" onclick="if (typeof window.captcha_image_audioObj !== 'undefined') captcha_image_audioObj.refresh(); document.getElementById('captcha_image').src = '?captcha=show&amp;' + Math.random(); this.blur(); return false">
                  <img height="32" width="32" src="{$captcha.securimage_uri}/images/refresh.png" alt="Refresh Image" onclick="this.blur()"/>
                </a>
                <div style="clear: both"></div>
              </div>
              Type the text:
              <input type="text" name="captcha_code" id="captcha_code" autocomplete="off"/>
            </label>
          {/if}
        </div>
        {if !$done}
        <div id="window-footer" class="plugbottom">
          <div>
          </div>
          <div>
            <input type="submit" id="form_submit" name="form_submit" value="{msgPool type=okButton}"/>
          </div>
        </div>
        {/if}
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
