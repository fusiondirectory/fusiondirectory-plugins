<h2>{t}Policy settings{/t}</h2>
<table style="width:100%">
 <tr>
  <td style="border-right:1px solid #AAA">
   <table>
    <tr>
      <td>{t}Policy name{/t}{$must}</td>
      <td><input type="text" name="name" value="{$name}"></td>
    </tr>
    <tr>
      <td>{t}Minimum password length{/t}</td>
      <td><input type="text" name="PW_MIN_LENGTH" value="{$PW_MIN_LENGTH}"></td>
    </tr>
    <tr>
      <td>{t}Required different characters{/t}</td>
      <td><input type="text" name="PW_MIN_CLASSES" value="{$PW_MIN_CLASSES}"></td>
    </tr>
    <tr>
      <td>{t}Password history size{/t}</td>
      <td><input type="text" name="PW_HISTORY_NUM" value="{$PW_HISTORY_NUM}"></td>
    </tr>
   </table>
   </td>
   <td style="vertical-align:top">
    <table>
     <td>
      <tr>
        <td>{t}Minimum password lifetime{/t}</td>
        <td><input type="text" name="PW_MIN_LIFE" value="{$PW_MIN_LIFE}">&nbsp;{t}seconds{/t}</td>
      </tr>
      <tr>
        <td>{t}Password lifetime{/t}</td>
        <td><input type="text" name="PW_MAX_LIFE" value="{$PW_MAX_LIFE}">&nbsp;{t}seconds{/t}</td>
      </tr>
    </table>
   </td>
  </tr>
  <tr>
    <td colspan="2"><br>{$POLICY_REFCNT}</td>
  </tr>
 </table>
<input type="hidden" name="Policy_Posted" value="1">
<p class="seperator">&nbsp;</p>

<div style="text-align:right; padding:4px;">
	<input type='submit' name="save_policy" value="{msgPool type=okButton}">
	&nbsp;
	<input type='submit' name="cancel_policy" value="{msgPool type=cancelButton}">
</div>
