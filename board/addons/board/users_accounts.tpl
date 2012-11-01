<div id="{$sectionId}"  class="plugin_section fullwidth">
  <span class="legend">
    {$section}
  </span>
  <div>
    <h1>
    {if $attributes.expired.accounts|@count > 0}
      {t count=$attributes.expired.accounts|@count 1=$attributes.expired.accounts|@count plural="There are %1 expired accounts"}There is one expired account{/t}
    {else}
      {t}There is no expired account{/t}
    {/if}
    </h1>
    {if $attributes.expired.accounts|@count > 0}
      <div class="listContainer">
      <table summary="" style="width: 100%; table-layout: fixed;"  cellpadding="4" cellspacing="0">
        <colgroup>
          <col width="10%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="10%"/>
        </colgroup>
        <thead class="fixedListHeader listHeaderFormat">
          <tr style="background-color: white; text-align:center;">
            <td colspan="4" class="listheader" style="background-color: white; padding:5px;">
              {t}Expired accounts{/t}
            </td>
            <td colspan="3" class="listheader" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </td>
          </tr>


          <tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
        <!-- uid/cn/telephonNumber/mail/shadowExpire/sambaKickoffTime -->
          <td class="listheader">{t}uid{/t}</td><td class="listheader">{t}cn{/t}</td><td class="listheader">{t}telephoneNumber{/t}</td><td class="listheader">{t}shadowExpire{/t}</td>
          <td class="listheader">{t}manager{/t}</td><td class="listheader">{t}mail{/t}</td><td class="listheader">{t}telephoneNumber{/t}</td>
          </tr>
        </thead>
        <tbody class="listScrollContent listBodyFormat">
          {foreach from=$attributes.expired.accounts item=account}
            <tr>
              <td class="list0">&nbsp;{$account.uid}</td>
              <td class="list0">&nbsp;{$account.cn}</td>
              <td class="list0">&nbsp;{$account.telephoneNumber}</td>
              <td class="list0">&nbsp;{$account.shadowExpire}</td>
              <td class="list0">&nbsp;{$account.manager_cn}</td>
              <td class="list0"><a href="mailto:{$account.manager_mail}">{$account.manager_mail}</a></td>
              <td class="list0">&nbsp;{$account.manager_phone}</td>
            </tr>
          {/foreach}
        </tbody>
      </table>
      </div>
     {/if}

    <h1>
    {if $attributes.expired.accounts_next_days|@count > 0}
      {t count=$attributes.expired.accounts_next_days|@count 1=$attributes.expired.next_days 2=$attributes.expired.accounts_next_days|@count plural="There are %2 accounts expiring in the next %1 days"}There is one account expiring in the next %1 days{/t}
    {else}
      {t 1=$attributes.expired.next_days}There is no account expiring in the next %1 days{/t}
    {/if}
    </h1>
    {if $attributes.expired.accounts_next_days|@count > 0}
      <div class="listContainer">
      <table summary="" style="width: 100%; table-layout: fixed;"  cellpadding="4" cellspacing="0">
        <colgroup>
          <col width="10%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="15%"/>
          <col width="10%"/>
        </colgroup>
        <thead class="fixedListHeader listHeaderFormat">
          <tr style="background-color: white; text-align:center;">
            <td colspan="4" class="listheader" style="background-color: white; padding:5px;">
              {t}Next expired accounts{/t}
            </th>
            <td colspan="3" class="listheader" style="background-color: white; padding:5px;">
              {t}Manager concerned{/t}
            </th>
          </tr>
          <tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
          <!-- uid/cn/telephonNumber/mail/shadowExpire/sambaKickoffTime -->
          <td class="listheader">{t}uid{/t}</td><td class="listheader">{t}cn{/t}</td><td class="listheader">{t}telephoneNumber{/t}</td><td class="listheader">{t}shadowExpire{/t}</td>
          <td class="listheader">{t}manager{/t}</td><td class="listheader">{t}mail{/t}</td><td class="listheader">{t}telephoneNumber{/t}</td>
          </tr>
        </thead>
        <tbody class="listScrollContent listBodyFormat">
          {foreach from=$attributes.expired.accounts_next_days item=account}
            <tr>
              <td class="list0">&nbsp;{$account.uid}</td>
              <td class="list0">&nbsp;{$account.cn}</td>
              <td class="list0">&nbsp;{$account.telephoneNumber}</td>
              <td class="list0">&nbsp;{$account.shadowExpire}</td>
              <td class="list0">&nbsp;{$account.manager_cn}</td>
              <td class="list0"><a href="mailto:{$account.manager_mail}">{$account.manager_mail}</a></td>
              <td class="list0">&nbsp;{$account.manager_phone}</td>
            </tr>
          {/foreach}
        </tbody>
      </table>
      </div>
    {/if}
  </div>
</div>
