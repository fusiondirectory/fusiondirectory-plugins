<div id="{$sectionId}"  class="plugin-section fullwidth">
  <span class="legend">
    {$section}
  </span>
  <div>
    <h1>
    {if $attributes.locked.accounts|@count >= 1}
      {t count=$attributes.locked.accounts|@count 1=$attributes.locked.accounts|@count plural="There are %1 locked accounts"}There is one locked account{/t}
    {else}
      {t}There is no locked account{/t}
    {/if}
    </h1>
    {if $attributes.locked.accounts|@count >= 1}
      <table style="width: 100%;" class="listingTable">
        <colgroup>
          <col width="10%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="10%"/>
        </colgroup>
        <thead>
          <tr style="background-color: white; text-align:center;">
            <th colspan="4" style="background-color: white; padding:5px;">
              {t}Locked accounts{/t}
            </th>
            <th colspan="3" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </th>
          </tr>


          <tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
        <!-- uid/cn/telephonNumber/mail/shadowExpire/sambaKickoffTime -->
          <th>{t}uid{/t}</th><th>{t}cn{/t}</th><th>{t}telephoneNumber{/t}</th><th>{t}pwdAccountLockedTime{/t}</th>
          <th>{t}manager{/t}</th><th>{t}mail{/t}</th><th>{t}telephoneNumber{/t}</th>
          </tr>
        </thead>
        <tbody>
          {foreach from=$attributes.locked.accounts item=account}
            <tr>
              <td>&nbsp;{$account.uid}</td>
              <td>&nbsp;{$account.cn}</td>
              <td>&nbsp;{$account.telephoneNumber}</td>
              <td>&nbsp;{$account.pwdAccountLockedTime}</td>
              <td>&nbsp;{$account.manager_cn}</td>
              <td><a href="mailto:{$account.manager_mail}">{$account.manager_mail}</a></td>
              <td>&nbsp;{$account.manager_phone}</td>
            </tr>
          {/foreach}
        </tbody>
      </table>
    {/if}
  </div>
</div>
