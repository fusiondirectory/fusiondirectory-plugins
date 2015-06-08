<table style="width:100%;height:100%; vertical-align:top; text-align:left;">
  <tr>
    <td style="vertical-align:top; width:100%;">
      <div class="contentboxh">
        <p class="contentboxh">&nbsp;{$HEADLINE}&nbsp;{$SIZELIMIT}</p>
      </div>

      <div class="contentboxb" style="background:white;">
        <table>
          <tbody>
            <tr>
              <td>{$ROOT}&nbsp;</td><td>{$BACK}&nbsp;</td><td>{$HOME}&nbsp;</td><td class="optional">{$RELOAD}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>
              {if $BASE}<td>{t}Base{/t} {$BASE}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>{/if}
              <td>{$ACTIONS}</td>
            </tr>
          </tbody>
        </table>
      </div>

      <div style='height:4px;'>
      </div>
      {$LIST}
    </td>
    <td style='vertical-align:top;min-width:250px'>
      {$FILTER}
    </td>
  </tr>
</table>

<p class="plugbottom">
  <input type=submit name="SaveService" value="{msgPool type=saveButton}">
</p>

<input type="hidden" name="ignore">
