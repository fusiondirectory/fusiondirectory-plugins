<script type="text/javascript" src="include/pwdStrength.js"></script>
<p>
 {t}To change the terminal root password use the fields below. The changes take effect during the next reboot. Please memorize the new password, because you wouldn't be able to log in.{/t}
</p>
<p>
 <b>{t}Leave fields blank for password inheritance from default entries.{/t}</b>
</p>

<p>
 {t}Changing the password impinges on authentification only.{/t}
</p>

<table summary="" style="vertical-align:top; text-align:left;" cellpadding=4 border=0>
  <tr>
    <td><b><LABEL for="new_password">{t}New password{/t}</LABEL></b></td>
    <td><input id="new_password" type="password" name="new_password" size="30" maxlength="40"
    onkeyup="testPasswordCss(document.getElementById('new_password').value);"
    onFocus="nextfield= 'repeated_password';"></td>
  </tr>
  <tr>
    <td><b><LABEL for="repeated_password">{t}Repeat new password{/t}</LABEL></b></td>
    <td><input type="password" id="repeated_password" name="repeated_password" size="30" maxlength="40"
    onFocus="nextfield= 'password_finish';"></td>
  </tr>
  <tr>
       <td>{t}Password strength{/t}</td>
       <td>
        <span id="meterEmpty" style="padding:0;margin:0;width:100%;background-color:#DC143C;display:block;height:5px;">
        <span id="meterFull" style="padding:0;margin:0;z-index:100;width:0;background-color:#006400;display:block;height:5px;"></span></span>
       </td>
      </tr>
 
</table>

<br>

<p class="plugbottom">
  <input type=submit name="password_finish" value="{t}Set password{/t}">
  &nbsp;
  <input type=submit name="password_cancel" value="{msgPool type=cancelButton}">
</p>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript"> 
  <!-- // First input field on page
    nextfield= 'new_password';
  focus_field('new_password');
  -->
</script>
