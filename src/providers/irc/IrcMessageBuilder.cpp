#include "providers/irc/IrcMessageBuilder.hpp"

#include "Application.hpp"
#include "common/IrcColors.hpp"
#include "controllers/accounts/AccountController.hpp"
#include "controllers/ignores/IgnoreController.hpp"
#include "controllers/ignores/IgnorePhrase.hpp"
#include "messages/Message.hpp"
#include "providers/chatterino/ChatterinoBadges.hpp"
#include "singletons/Emotes.hpp"
#include "singletons/Resources.hpp"
#include "singletons/Settings.hpp"
#include "singletons/Theme.hpp"
#include "singletons/WindowManager.hpp"
#include "util/Helpers.hpp"
#include "util/IrcHelpers.hpp"
#include "widgets/Window.hpp"

namespace chatterino {

IrcMessageBuilder::IrcMessageBuilder(
    Channel *_channel, const Communi::IrcPrivateMessage *_ircMessage,
    const MessageParseArgs &_args)
    : SharedMessageBuilder(_channel, _ircMessage, _args)
{
}

IrcMessageBuilder::IrcMessageBuilder(Channel *_channel,
                                     const Communi::IrcMessage *_ircMessage,
                                     const MessageParseArgs &_args,
                                     QString content, bool isAction)
    : SharedMessageBuilder(_channel, _ircMessage, _args, content, isAction)
{
    assert(false);
}

IrcMessageBuilder::IrcMessageBuilder(
    const Communi::IrcNoticeMessage *_ircMessage, const MessageParseArgs &_args)
    : SharedMessageBuilder(Channel::getEmpty().get(), _ircMessage, _args,
                           _ircMessage->content(), false)
{
}

MessagePtr IrcMessageBuilder::build()
{
    // PARSE
    this->parse();
    this->usernameColor_ = getRandomColor(this->ircMessage->nick());

    // PUSH ELEMENTS
    this->appendChannelName();

    this->message().serverReceivedTime = calculateMessageTime(this->ircMessage);
    this->emplace<TimestampElement>(this->message().serverReceivedTime.time());

    this->appendUsername();

    // message
    this->addIrcMessageText(this->originalMessage_);

    this->message().searchText = this->message().localizedName + " " +
                                 this->userName + ": " + this->originalMessage_;

    // highlights
    this->parseHighlights();

    // highlighting incoming whispers if requested per setting
    if (this->args.isReceivedWhisper && getSettings()->highlightInlineWhispers)
    {
        this->message().flags.set(MessageFlag::HighlightedWhisper, true);
    }

    return this->release();
}

void IrcMessageBuilder::appendUsername()
{
    QString username = this->userName;
    this->message().loginName = username;
    this->message().displayName = username;

    // The full string that will be rendered in the chat widget
    QString usernameText = username;

    if (this->args.isReceivedWhisper)
    {
        this->emplace<TextElement>(usernameText, MessageElementFlag::Username,
                                   this->usernameColor_,
                                   FontStyle::ChatMediumBold)
            ->setLink({Link::UserWhisper, this->message().displayName});

        // Separator
        this->emplace<TextElement>("->", MessageElementFlag::Username,
                                   MessageColor::System, FontStyle::ChatMedium);

        this->emplace<TextElement>("you:", MessageElementFlag::Username);
    }
    else
    {
        if (!this->action_)
        {
            usernameText += ":";
        }
        this->emplace<TextElement>(usernameText, MessageElementFlag::Username,
                                   this->usernameColor_,
                                   FontStyle::ChatMediumBold)
            ->setLink({Link::UserInfo, this->message().loginName});
    }
}

}  // namespace chatterino
