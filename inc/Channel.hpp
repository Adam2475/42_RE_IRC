#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "header.hpp"
# include "User.hpp"

class Channel
{
	private:
		std::vector<User> _user_vector;
		std::vector<User> _operators_vector;
		std::string		_name;
		std::string		_passwd;
		std::string		_topic;
		size_t	_max_users;

		// list of invited users
		std::vector<User> _invited_users;

		// MODE necessities
		bool		_invite_only;
		bool		_topic_restriction;
		
	public:
		Channel();
		Channel(const Channel& other);
		Channel& operator=(const Channel& other);
		Channel(std::string& name, std::string& passwd, User& creator, std::string& topic
			, size_t max_users, bool invite_only, bool topic_restriction);
		~Channel();

		void	addUserToChannel(User& user, std::string& passwd);
		// void	inviteUser(User& user, User&user_operator);
		void	addUserToOperatorsVector(User& user, User& user_operator);
		// void	removeUserFromVector(User& user, std::vector<User>& vector);
		// void	setMaxUsers(size_t max_users, User& user_operator);
		/////////////////////////////////////
		// Channel Menagement
		// void	kickUser(User& user, User& user_operator, std::string msg);
		// void	partUser(User& user, Channel &channel, std::string msg);
		void	writeToChannel(std::string& buffer) const;
		// void	showChannelTopic();
		// bool	isOperatorUser(User target_user) const;
		// void	inviteUser(User& user, User& user_operator);
		////////////////////////////////////
		// Getters & Setters

		std::vector<User> getUserVector() const;
		std::vector<User> getInvitedUsersVector() const;
		std::vector<User> getUserOperatorsVector() const;
		std::string getName() const;
		std::string	getTopic() const;
		size_t  getMaxUsers() const;
		bool 	getInviteOnly() const;
		bool 	getTopicRestriction() const;
		void	setInviteOnly(bool set);
		void	setTopicRestriction(bool set);
		void	setPassword(std::string& pass);
		void	setMaxUsers(size_t num);
		void	setName(std::string& name);
		void	setTopic(std::string& topic);

		/////////////////////

		// void	addToInvited(User& user);
		std::string getNickList() const;

		/////////////////////


		// void modeInvite(std::string& arg);
		// void modePassword(std::stringstream& oss, std::string& arg);
		// void modeMaxUsers(std::stringstream& oss, std::string& arg);
		// void modeOperator(std::stringstream& oss, User& user, std::string& arg);
		// void modeTopic(std::stringstream& oss, std::string& arg);
};

#endif