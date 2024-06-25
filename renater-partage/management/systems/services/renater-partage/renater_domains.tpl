<fieldset id="{$sectionId}" class="plugin-section{$sectionClasses}">
  <legend><span><label for="{$attributes.fdRenaterPartageServerMailDomain.htmlid}">{if !empty($sectionIcon)}<img src="{$sectionIcon|escape}" alt=""/>{/if}{$section|escape}</label></span></legend>
  <div>
  <table>
    <tr class="
      {if $attributes.fdRenaterPartageServerMailDomain.subattribute}subattribute{/if}
      {if $attributes.fdRenaterPartageServerMailDomain.required}required{/if}
      {if !$attributes.fdRenaterPartageServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdRenaterPartageServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.fdRenaterPartageServerMailDomain.description|escape}" colspan="4">
        {eval var=$attributes.fdRenaterPartageServerMailDomain.input}
      </td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdRenaterPartageServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdRenaterPartageServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.renaterPartageDomain.description|escape}">
        <label for="{$attributes.renaterPartageDomain.htmlid}">
          {eval var=$attributes.renaterPartageDomain.label}
        </label>
      </td>
      <td>{eval var=$attributes.renaterPartageDomain.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdRenaterPartageServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdRenaterPartageServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.renaterPartageDomainKey.description|escape}">
        <label for="{$attributes.renaterPartageDomainKey.htmlid}">
          {eval var=$attributes.renaterPartageDomainKey.label}
        </label>
      </td>
      <td>{eval var=$attributes.renaterPartageDomainKey.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdRenaterPartageServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdRenaterPartageServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.renaterPartageDomainCOS.description|escape}">
        <label for="{$attributes.renaterPartageDomainCOS.htmlid}">
          {eval var=$attributes.renaterPartageDomainCOS.label}
        </label>
      </td>
      <td>{eval var=$attributes.renaterPartageDomainCOS.input}</td>
    </tr>
    <tr class="
      subattribute
      {if !$attributes.fdRenaterPartageServerMailDomain.readable}nonreadable{/if}
      {if !$attributes.fdRenaterPartageServerMailDomain.writable}nonwritable{/if}
    ">
      <td title="{$attributes.fdRenaterPartageServerMailDomain_buttons.description|escape}" colspan="4">
        {eval var=$attributes.fdRenaterPartageServerMailDomain_buttons.input}
      </td>
    </tr>
  </table>
  </div>
</fieldset>
