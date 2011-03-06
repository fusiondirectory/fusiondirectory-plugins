{if $object == "server"}
    <table summary="">
     <tr>
      <td><LABEL for="cn">{t}Server name{/t}</LABEL>{$must}</td>
      <td>
       <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
      </td>
     </tr>
	</table>
{/if}
{if $object == "workstation"}
    <table summary="">
     <tr>
      <td><LABEL for="cn">{t}workstation name{/t}</LABEL>{$must}</td>
      <td>
       <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
      </td>
     </tr>
	</table>
{/if}
{if $object == "terminal"}
    <table summary="">
     <tr>
      <td><LABEL for="cn">{t}Terminal name{/t}</LABEL>{$must}</td>
      <td>
       <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
      </td>
     </tr>
	</table>
{/if}
{if $object == "printer"}
    <table summary="">
     <tr>
      <td><LABEL for="cn">{t}Printer name{/t}</LABEL>{$must}</td>
      <td>
       <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
      </td>
     </tr>
	</table>
{/if}
{if $object == "component"}
    <table summary="">
     <tr>
      <td><LABEL for="cn">{t}Component name{/t}</LABEL>{$must}</td>
      <td>
       <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
      </td>
     </tr>
	</table>
{/if}
