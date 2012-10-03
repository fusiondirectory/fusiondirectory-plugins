    <table summary="">
     <tr>
      <td>
        <LABEL for="cn">
{if     $object == "server"}
          {t}Server name{/t}
{elseif $object == "workstation"}
          {t}workstation name{/t}
{elseif $object == "terminal"}
          {t}Terminal name{/t}
{elseif $object == "printer"}
          {t}Printer name{/t}
{elseif $object == "component"}
          {t}Component name{/t}
{elseif $object == "phone"}
          {t}Phone name{/t}
{/if}
        </LABEL>{$must}
      </td>
      <td>
       <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}" />
      </td>
     </tr>
     <tr>
      <td><LABEL for="macAddress">{t}Mac address{/t}</LABEL>{$must}</td>
      <td>
       <input type='text' name="macAddress" id="macAddress" size=20 maxlength=60 value="{$macAddress}" />
      </td>
     </tr>
     <tr>
      <td><LABEL for="ipHostNumber">{t}IP{/t}</LABEL></td>
      <td>
       <input type='text' name="ipHostNumber" id="ipHostNumber" size=20 maxlength=60 value="{$ipHostNumber}" />
      </td>
     </tr>
  </table>
