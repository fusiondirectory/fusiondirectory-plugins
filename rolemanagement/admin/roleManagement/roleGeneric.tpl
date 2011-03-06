<table style='width:100%;'>
  <tr>
    <td style='width:50%; vertical-align:top; border-right:1px solid #A0A0A0; padding-right:10px;'>

      <h2><img src='plugins/rolemanagement/images/role.png' alt='' class='center'>&nbsp;
        {t}Generic{/t}
      </h2>

      <table style='width:100%;'>
        <tr>
          <td>{t}Name{/t}{$must}</td>
          <td>
            {render acl=$cnACL}
             <input type='text' value='{$cn}' name='cn'>
            {/render}
          </td>
        </tr>
        <tr>
          <td>{t}Description{/t}</td>
          <td>
            {render acl=$descriptionACL}
             <input type='text' value='{$description}' name='description'>
            {/render}
          </td>
        </tr>
        <tr>
          <td>
            <div style="height:10px;"></div>
            <label for="base">{t}Base{/t}</label>
          </td>
          <td>
            <div style="height:10px;"></div>
      {render acl=$baseACL}
            {$base}
      {/render}
          </td>
        </tr>
        <tr>
          <td colspan="2"><p class="seperator">&nbsp;</p><br></td>
        </tr>
        <tr>
          <td>{t}Phone number{/t}</td>
          <td>
            {render acl=$telephoneNumberACL}
             <input type='text' value='{$telephoneNumber}' name='telephoneNumber'>
            {/render}
          </td>
        </tr>
        <tr>
          <td>{t}Fax number{/t}</td>
          <td>
            {render acl=$facsimileTelephoneNumberACL}
             <input type='text' value='{$facsimileTelephoneNumber}' name='facsimileTelephoneNumber'>
            {/render}
          </td>
        </tr>
      </table>

    </td>
    <td style=' vertical-align:top; padding-left:10px;'>
      <h2><img src='plugins/rolemanagement/images/occupant.png' alt='' class='center'>&nbsp;
        {t}Occupants{/t}
      </h2>

{render acl=$roleOccupantACL}
      <select style="width:100%; height:450px;" id="members" name="members[]" size="15" multiple>
        {$members}
      </select>
{/render}
      <br>
{render acl=$roleOccupantACL}
      <input type=submit value="{msgPool type=addButton}" name="edit_membership">&nbsp;
{/render}
{render acl=$roleOccupantACL}
      <input type=submit value="{msgPool type=delButton}" name="delete_membership">
{/render}
    </td>
  </tr>
</table>  
