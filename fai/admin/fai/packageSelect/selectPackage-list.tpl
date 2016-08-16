<table class="listing_container">
  <tbody>
    <tr>
      <td class="list">
        <div class="contentboxh">
          <p class="contentboxh">&nbsp;{$HEADLINE}&nbsp;{$SIZELIMIT}</p>
        </div>

        <div class="contentboxb" style="background:white;">
          <table>
            <tbody>
              <tr>
                <td>{$ROOT}&nbsp;</td><td>{$BACK}&nbsp;</td><td>{$HOME}&nbsp;</td><td class="optional">{$RELOAD}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>
                <td><input class="center" type="image" src="geticon.php?context=actions&amp;icon=submit&amp;size=16" title="{t}Update{/t}" name="submit_department" alt="{t}Submit{/t}"/>&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>
                <td>{$ACTIONS}</td>
              </tr>
            </tbody>
          </table>
        </div>

        <div style="margin-top:4px;">
          {$LIST}
        </div>
      </td>
      <td class="filter">
        {$FILTER}
      </td>
    </tr>
  </tbody>
</table>

<p class="plugbottom">
  <input type="submit" name="packageSelect_save" value="{msgPool type=addButton}"/>
  &nbsp;
  <input type="submit" name="packageSelect_cancel" value="{msgPool type=cancelButton}"/>
</p>

<input type="hidden" name="ignore"/>
