include $(TOPDIR)/rules.mk

PKG_NAME:=liboauth
PKG_VERSION:=0.7.1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://liboauth.sourceforge.net/pool/
#PKG_MD5SUM:=9b7dc52656f5cbec846a7ba3299f73bd
PKG_CAT:=zcat
PKG_MAINTAINER:=Robin Gareus <robin@gareus.org>

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/liboauth
  SECTION:=libs
  CATEGORY:=Libraries
  DEFAULT:=y
  DEPENDS:=+libcurl
  TITLE:=liboauth is a collection of POSIX-c functions implementing the OAuth Core RFC 5849 standard
  #DESCRIPTION:=This variable is obsolete. use the Package/name/description define instead!
  URL:=http://liboauth.sourceforge.net/
endef

define Package/liboauth/description
liboauth is a collection of POSIX-c functions implementing the OAuth Core RFC 5849 standard
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
endef

define Package/liboauth/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liboauth.{a,so*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,liboauth))
