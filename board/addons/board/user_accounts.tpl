<div class="plugin_section">
  <span class="legend">
    {t}Statistics{/t}
  </span>
  <div>
    <img src="{$user_img}" alt="user icon"/>
    {t count=$nb_accounts 1=$nb_accounts plural="There are %1 users:"}There is 1 user:{/t}
    <ul>
      {foreach from=$nb_type_accounts item=acc}
        <li style="list-style-image:url({$acc.img})">
        {if $acc.nb > 0}
          {t count=$acc.nb 1=$acc.name 2=$acc.nb plural="%2 of them have a %1 account"}One of them have a %1 account{/t}
        {else}
          {t 1=$acc.name}None of them have a %1 account{/t}
        {/if}
        </li>
      {/foreach}
      <li style="list-style-image:url({$locked_accounts.img})">
        {if $locked_accounts.nb > 0}
          {t count=$locked_accounts.nb 1=$locked_accounts.nb plural="%1 of them are locked"}One of them is locked{/t}
        {else}
          {t}None of them is locked{/t}
        {/if}
      </li>
    </ul>
  </div>
</div>
<div class="plugin_section" style="width:98%;">
  <span class="legend">
    {t}Expired accounts{/t}
  </span>
  <div>
    <h1>
    {if $expired_accounts|@count > 0}
      {t count=$expired_accounts|@count 1=$expired_accounts|@count plural="There are %1 expired accounts"}There is one expired account{/t}
    {else}
      {t}There is no expired account{/t}
    {/if}
    </h1>
    {if $expired_accounts|@count > 0}
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
          {foreach from=$expired_accounts item=account}
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
    {if $expired_accounts_in_next_days|@count > 0}
      {t count=$expired_accounts_in_next_days|@count 1=$next_expired_days 2=$expired_accounts_in_next_days|@count plural="There are %2 accounts expiring in the next %1 days"}There is one account expiring in the next %1 days{/t}
    {else}
      {t 1=$next_expired_days}There is no account expiring in the next %1 days{/t}
    {/if}
    </h1>
    {if $expired_accounts_in_next_days|@count > 0}
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
          {foreach from=$expired_accounts_in_next_days item=account}
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
